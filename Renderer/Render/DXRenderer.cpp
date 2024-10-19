#include "stdafx.h"

#include "DXRenderer.h"

#include "DXObjects/GraphicsCommandList.h"
#include "Events/MouseScrollEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/ResizeEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "Render/TaskGPU.h"
#include "Utility/DebugInfo.h"

#include "GUI/GUI.h"

using namespace DirectX;
using namespace Core;

namespace
{
    constexpr float MOVE_SPEED = 200.0f;
} // namespace unnamed

DXRenderer::DXRenderer(HWND windowHandle)
    : _windowHandle(windowHandle)
    , _contentLoaded(false)
    , _ambient(nullptr)
    , _isCameraMoving(false)
    , _deltaTime(0.0f)
{   }

DXRenderer::~DXRenderer()
{   }

bool DXRenderer::LoadContent(TaskGPU* loadTask)
{
    _gPassPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    _deferredPipeline.Parse("PipelineDescriptions\\DeferredShading.tech");
    _renderPipeline.Parse("PipelineDescriptions\\TriangleRenderPipeline.tech");
    _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");

    _gBuffer.Init({ 1280, 720 });

    // TODO: rework
    {
        ComPtr<ID3DBlob> computeShaderBlob = nullptr;
        Helper::throwIfFailed(D3DReadFileToBlob(L"NegativePostFX_cs.cso", &computeShaderBlob));

        D3D12_INPUT_ELEMENT_DESC* inputLayout = nullptr;

        D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateStreamDesc = {};

        Helper::throwIfFailed(Core::Device::GetDXDevice()->CreateRootSignature(0, computeShaderBlob->GetBufferPointer(),
            computeShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_postFXRootSig)));

        pipelineStateStreamDesc.CS = CD3DX12_SHADER_BYTECODE(computeShaderBlob.Get());

        Helper::throwIfFailed(Core::Device::GetDXDevice()->CreateComputePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_postFXPipeState)));
    }

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(-30.0f, 40.0f, -50.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        RECT windowSize;
        GetWindowRect(_windowHandle, &windowSize);
        float width = windowSize.right - windowSize.left;
        float height = windowSize.bottom - windowSize.top;
        _camera.SetViewport(SceneLayer::Viewport({ width, height }));

        _camera.LookAt(pos, target, up);
        _camera.SetLens(45.0f, 0.1f, 1000.0f);
    }

    // Load scene
    {
        loadTask->SetName("Upload Data");
        Core::GraphicsCommandList* commandList = loadTask->GetCommandLists().front();

        _scene.LoadScene("Wyvern\\Wyvern.scene", *commandList);

        _scene.SetCamera(_camera);

        commandList->Close();

        std::vector<ID3D12CommandList*> comLists;
        comLists.reserve(loadTask->GetCommandLists().size());
        for (auto cl : loadTask->GetCommandLists())
        {
            comLists.push_back(cl->GetDXCommandList().Get());
        }

        loadTask->GetCommandQueue()->ExecuteCommandLists(comLists.size(), comLists.data());
        loadTask->GetCommandQueue()->Signal(loadTask->GetFence()->GetFence().Get(), loadTask->GetFenceValue());
    }

    _contentLoaded = true;
    return _contentLoaded;
}

void DXRenderer::UnloadContent()
{
    _contentLoaded = false;
}

void DXRenderer::OnUpdate(Events::UpdateEvent& updateEvent)
{
    DebugInfo::Update(updateEvent);

    _deltaTime = updateEvent.elapsedTime;
}

void DXRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
{
    frame.WaitCPU();
    frame.ResetGPU();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = frame._targetHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = frame._depthHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear render targets
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        Core::GraphicsCommandList& commandList = *task->GetCommandLists().front();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Clean");
        {
            commandList.TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

            FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

            commandList.ClearRTV(rtv, clearColor);
            commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    // Execute the TriangleRender shader
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_gPassPipeline);
        task->SetName("g-pass");
        task->AddDependency("clean");

        Core::GraphicsCommandList& commandList = *task->GetCommandLists().front();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 18, "Geometry Pass");
        {
            _gBuffer.ClearTextures(commandList);

            commandList.SetPipelineState(_gPassPipeline);
            commandList.SetGraphicsRootSignature(_gPassPipeline);

            commandList.SetViewport(_camera.GetViewport());
            commandList.SetRenderTargets({ _gBuffer.GetPositionTextureCPUHandle(), _gBuffer.GetAlbedoMetalnessTextureCPUHandle(), _gBuffer.GetNormalTextureCPUHandle() }, &dsv);

            _scene.Draw(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    // Execute the TriangleRender shader
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_deferredPipeline);
        task->SetName("deferred");
        task->AddDependency("g-pass");

        Core::GraphicsCommandList& commandList = *task->GetCommandLists().front();

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
        {
#if defined(_DEBUG)
            DebugInfo::StartStatCollecting(commandList);
#endif
            commandList.SetPipelineState(_deferredPipeline);
            commandList.GetDXCommandList()->SetComputeRootSignature(_deferredPipeline.GetRootSignature().Get());

            _scene.DeferredDraw(commandList);

            commandList.TransitionBarrier(_gBuffer.GetPositionTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            D3D12_CPU_DESCRIPTOR_HANDLE handle = frame._postFXDescHeap.GetHeapStartCPUHandle();
            handle.ptr += 32;
            Core::Device::GetDXDevice()->CopyDescriptorsSimple(3, handle, _gBuffer.GetUAVHeap().GetHeapStartCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            ID3D12DescriptorHeap* heap[1] = { frame._postFXDescHeap.GetDXDescriptorHeap().Get() };
            commandList.GetDXCommandList()->SetDescriptorHeaps(1, heap);

            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = frame._postFXDescHeap.GetHeapStartGPUHandle();
            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(6, gpuHandle);
            gpuHandle.ptr += 32;
            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(3, gpuHandle);
            gpuHandle.ptr += 32;
            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(4, gpuHandle);
            gpuHandle.ptr += 32;
            commandList.GetDXCommandList()->SetComputeRootDescriptorTable(5, gpuHandle);

            commandList.GetDXCommandList()->Dispatch(1280, 720, 1);

            commandList.TransitionBarrier(_gBuffer.GetPositionTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

#if defined(_DEBUG)
            //commandList.SetPipelineState(_AABBpipeline);
            //commandList.SetGraphicsRootSignature(_AABBpipeline);
            //commandList.SetConstants(1, sizeof(XMMATRIX) / 4, &_camera.ViewProjection());

            //_scene.DrawAABB(commandList);

            DebugInfo::EndStatCollecting(commandList);
#endif
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    //GUI
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        task->AddDependency("deferred");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();

        PIXBeginEvent(commandList->GetDXCommandList().Get(), 7, "GUI");
        {
            commandList->SetViewport(_camera.GetViewport());
            commandList->SetRenderTarget(&rtv, &dsv);

            if (ImGui::Begin("Debug Info"), true, ImGuiWindowFlags_AlwaysAutoResize)
            {
                ImGui::SetWindowPos({ 0, 0 });
                ImGui::SetWindowSize({ 0, 0 });

                ImGui::Text("FPS: %i (%.03f ms)", DebugInfo::GetFPS(), DebugInfo::GetMsPerFrame());

                if (ImGui::CollapsingHeader("Pipeline statistics"))
                {
                    D3D12_QUERY_DATA_PIPELINE_STATISTICS stats = DebugInfo::GetPipelineStatisctics();
                    ImGui::Text(std::string("Primitives: " + std::to_string(stats.IAPrimitives)).c_str());
                    ImGui::Text(std::string("VS invocs: " + std::to_string(stats.VSInvocations)).c_str());
                    ImGui::Text(std::string("GS invocs: " + std::to_string(stats.GSInvocations)).c_str());
                    ImGui::Text(std::string("PS invocs: " + std::to_string(stats.PSInvocations)).c_str());
                }

                if (ImGui::CollapsingHeader("Inputs"))
                {
                    ImGuiIO& io = ImGui::GetIO();
                    if (ImGui::IsMousePosValid())
                    {
                        ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
                    }
                    else
                    {
                        ImGui::Text("Mouse pos: <INVALID>");
                    }
                    ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
                    ImGui::Text("Mouse down:");
                    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
                    {
                        if (ImGui::IsMouseDown(i))
                        {
                            ImGui::SameLine();
                            ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
                        }
                    }

                    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key >= 0 && key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
                    ImGuiKey start_key = (ImGuiKey)0;

                    ImGui::Text("Keys down:");         for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) { if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) continue; ImGui::SameLine(); ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); }
                    ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
                    ImGui::Text("Chars queue:");       for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
                }
            }
            ImGui::End();

            GUI::Render(*commandList);
        }
        PIXEndEvent(commandList->GetDXCommandList().Get());

        commandList->Close();
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("gui");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();

        PIXBeginEvent(commandList->GetDXCommandList().Get(), 5, "Present");
        {
            commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
            commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
            commandList->CopyResource(frame._targetTexture, frame._swapChainTexture);
            commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
        }
        PIXEndEvent(commandList->GetDXCommandList().Get());

        commandList->Close();
    }
}

void DXRenderer::OnKeyPressed(Events::KeyEvent& e)
{
    XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    if (e.keyCode == DIKeyCode::DIK_W)
    {
        dir += _camera.Look() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_S)
    {
        dir -= _camera.Look() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_D)
    {
        dir += _camera.Right() * _deltaTime * MOVE_SPEED;
    }
    if (e.keyCode == DIKeyCode::DIK_A)
    {
        dir -= _camera.Right() * _deltaTime * MOVE_SPEED;
    }
    _camera.Update(dir);

    switch (e.keyCode)
    {
    case DIKeyCode::DIK_ESCAPE:
        ::SendMessage(_windowHandle, WM_DESTROY, 0, 0);
        break;
    }
}

void DXRenderer::OnMouseMoved(Events::MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
    {
        _camera.Update(e.relativeX, e.relativeY);
    }
}

void DXRenderer::OnMouseButtonPressed(Events::MouseButtonEvent& e)
{
    if (e.rightButton)
    {
        _isCameraMoving = true;
    }
}

void DXRenderer::OnMouseButtonReleased(Events::MouseButtonEvent& e)
{
    if (!e.rightButton)
    {
        _isCameraMoving = false;
    }
}
