#include "stdafx.h"

#include "DXRenderer.h"

#include "CommandList.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"
#include "Events/KeyEvent.h"
#include "GUI/GUI.h"
#include "Render/Frame/TaskGPU.h"
#include "Utility/DebugInfo.h"

#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Transformation.h"
#include "Scene/ECS/Components/Skybox.h"

using namespace DirectX;
using namespace Core;

namespace
{
    constexpr float MOVE_SPEED = 200.0f;
} // namespace unnamed

DXRenderer::DXRenderer(HWND windowHandle)
    : _windowHandle(windowHandle)
    , _contentLoaded(false)
    , _currentFrame(nullptr)
    , _isCameraMoving(false)
    , _deltaTime(0.0f)
{   }

DXRenderer::~DXRenderer()
{   }

bool DXRenderer::LoadContent(TaskGPU* loadTask)
{
    _gPassPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    _deferredPipeline.Parse("PipelineDescriptions\\DeferredShading.tech");
    _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");
    _SkyboxPipeline.Parse("PipelineDescriptions\\SkyboxPipeline.tech");
    _ArmatureDebugPipeline.Parse("PipelineDescriptions\\ArmatureDebugPipeline.tech");

    RECT windowSize;
    GetWindowRect(_windowHandle, &windowSize);
    uint32_t windowWidth = windowSize.right - windowSize.left;
    uint32_t windowHeight = windowSize.bottom - windowSize.top;

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(0.0f, 30.0f, 30.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        
        _camera.LookAt(pos, target, up);
        _camera.SetViewport(SceneLayer::Viewport({ windowWidth, windowHeight }));
        _camera.SetLens(45.0f, 0.1f, 1000.0f);
    }

    _gBuffer.Init({ windowWidth, windowHeight });

    // Load scene
    {
        loadTask->SetName("Upload Data");
        dx12::CommandList& commandList = *loadTask->GetCommandLists().front();

        _scene.LoadScene("Dragon\\DragonScene.scene", commandList);
        _uploadProcessor.Process(_scene, commandList);
        _scene.SetCamera(_camera);

        commandList.Close();
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

    _scene.GetCache().SetTime(updateEvent.totalTime);

    XMVECTOR mov = 37.0f * XMVectorSet(sinf(updateEvent.totalTime * 0.5f), 0.8f, cosf(updateEvent.totalTime * 0.5f), 1.0f);
    XMVECTOR tar = XMVectorSet(0.0f, 17.0f, 0.0f, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    _camera.LookAt(mov, tar, up);

    _deltaTime = updateEvent.elapsedTime;
}

void DXRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
{
    _currentFrame = &frame;

    _currentFrame->WaitCPU();
    _currentFrame->ResetGPU();

    // Clear render targets
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("clean");

        ClearBuffers(*task);
    }

    // Execute the GBuffer Pass
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_gPassPipeline);
        task->SetName("g-pass");
        task->AddDependency("clean");

        GeometryPass(*task);
    }

    // Execute the Deferred Shading
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_deferredPipeline);
        task->SetName("deferred");
        task->AddDependency("g-pass");

        LightingPass(*task);
    }

    // Execute the Skybox
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, &_SkyboxPipeline);
        task->SetName("skybox");
        task->AddDependency("g-pass");

        RenderSkybox(*task);
    }

    // Render Armature
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_ArmatureDebugPipeline);
        task->SetName("armature");
        task->AddDependency("deferred");
        task->AddDependency("skybox");

        RenderArmature(*task);        
    }

    //GUI
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        task->AddDependency("armature");

        RenderGUI(*task);
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("gui");

        Present(*task);
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

void DXRenderer::OnResize(Core::Events::ResizeEvent& e)
{
    Frame* current = _currentFrame;
    do
    {
        current->WaitCPU();
        current->ResetGPU();
        current = current->Next;
    } while (current != _currentFrame);

    DirectX::XMUINT2 windowSize = { (uint32_t)e.width, (uint32_t)e.height };

    do
    {
        current->Resize(windowSize);
        current = current->Next;
    } while (current != _currentFrame);

    _camera.SetViewport(SceneLayer::Viewport(windowSize));
    _gBuffer.Init(windowSize);
}

void DXRenderer::ClearBuffers(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = _currentFrame->_targetHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 1, "Clean");
    {
        commandList.TransitionBarrier(_currentFrame->_targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList.ClearRTV(rtv, clearColor);
        commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::GeometryPass(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Geometry Pass");
    {
        _gBuffer.ClearTextures(commandList);

        commandList.SetPipelineState(_gPassPipeline);
        commandList.SetRootSignature(_gPassPipeline);

        commandList.SetViewport(_camera.GetViewport());
        commandList.SetRenderTargets({ _gBuffer.GetAlbedoMetalnessTextureCPUHandle(), _gBuffer.GetNormalTextureCPUHandle() }, &dsv);

#if defined(_DEBUG)
        DebugInfo::StartStatCollecting(commandList);
#endif

        _cachedDataProcessor.Process(_scene, commandList);
        _drawProcessor.Process(_scene, commandList);

#if defined(_DEBUG)
        DebugInfo::EndStatCollecting(commandList);
#endif

        commandList.TransitionBarrier(_currentFrame->_depthTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::LightingPass(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
    {
        commandList.SetPipelineState(_deferredPipeline);
        commandList.SetRootSignature(_deferredPipeline);

        _cachedDataProcessor.Process(_scene, commandList);

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _currentFrame->_postFXDescHeap.GetHeapStartCPUHandle();
        handle.ptr += 64;
        dx12::Device::GetDXDevice()->CopyDescriptorsSimple(2, handle, _gBuffer.GetUAVHeap().GetHeapStartCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        ID3D12DescriptorHeap* heap[1] = { _currentFrame->_postFXDescHeap.GetDXDescriptorHeap().Get() };
        commandList.GetDXCommandList()->SetDescriptorHeaps(1, heap);

        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _currentFrame->_postFXDescHeap.GetResourceGPUHandle(&_currentFrame->_targetTexture);
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(6, gpuHandle);
        gpuHandle = _currentFrame->_postFXDescHeap.GetResourceGPUHandle(&_currentFrame->_depthTexture);
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(3, gpuHandle);
        gpuHandle = _currentFrame->_postFXDescHeap.GetHeapStartGPUHandle();
        gpuHandle.ptr += 64;
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(4, gpuHandle);
        gpuHandle.ptr += 32;
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(5, gpuHandle);

        DirectX::XMUINT2 viewportSize = _camera.GetViewport().GetSize();
        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::RenderSkybox(TaskGPU& task)
{
    std::shared_ptr<SceneLayer::Entity> entity = _scene.FindNodeByComponentName("Skybox");
    if (!entity)
    {
        return;
    }

    Skybox* skybox = entity->GetComponentAs<Skybox>("Skybox");

    dx12::CommandList& commandList = *task.GetCommandLists().front();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "Skybox");
    {
        commandList.SetPipelineState(_SkyboxPipeline);
        commandList.SetRootSignature(_SkyboxPipeline);

        _cachedDataProcessor.Process(_scene, commandList);

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _currentFrame->_testHeap.GetHeapStartCPUHandle();
        handle.ptr += 64;
        dx12::Device::GetDXDevice()->CopyDescriptorsSimple(1, handle, skybox->DescHeap.GetHeapStartCPUHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        ID3D12DescriptorHeap* heap[1] = { _currentFrame->_testHeap.GetDXDescriptorHeap().Get() };
        commandList.GetDXCommandList()->SetDescriptorHeaps(1, heap);

        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _currentFrame->_testHeap.GetResourceGPUHandle(&_currentFrame->_depthTexture);
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(3, gpuHandle);
        gpuHandle = _currentFrame->_testHeap.GetResourceGPUHandle(&_currentFrame->_targetTexture);
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(5, gpuHandle);
        gpuHandle = _currentFrame->_testHeap.GetHeapStartGPUHandle();
        gpuHandle.ptr += 64;
        commandList.GetDXCommandList()->SetComputeRootDescriptorTable(4, gpuHandle);

        DirectX::XMUINT2 viewportSize = _camera.GetViewport().GetSize();
        int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
        int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

        commandList.GetDXCommandList()->Dispatch(xThreadGroups, yThreadGroups, 1);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::RenderArmature(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "Armature");
    {
        commandList.SetPipelineState(_ArmatureDebugPipeline);
        commandList.SetRootSignature(_ArmatureDebugPipeline);

        for (auto& node : _scene.GetRootNodes())
        {
            Armature* arm = node->GetComponentAs<Armature>("Armature");
            Transformation* transform = node->GetComponentAs<Transformation>("Transformation");

            if (arm)
            {
                DirectX::XMVECTOR* data = (DirectX::XMVECTOR*)arm->BoneDebugTransforms.Map();

                const auto& sortedBones = arm->GetSortedBones();
                int ind = 0;
                for (const auto& bone : sortedBones)
                {
                    for (const auto& child : bone->Children)
                    {
                        data[ind++] = DirectX::XMVector4Transform(bone->GlobalTransform.r[3], transform->Transform);
                        data[ind++] = DirectX::XMVector4Transform(child->GlobalTransform.r[3], transform->Transform);
                    }
                }

                D3D12_CPU_DESCRIPTOR_HANDLE rtv = _currentFrame->_targetHeap->GetCPUDescriptorHandleForHeapStart();
                D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();

                commandList.SetViewport(_camera.GetViewport());
                commandList.SetRenderTarget(&rtv, &dsv);

                DirectX::XMMATRIX vp = _camera.ViewProjection();

                commandList.SetConstants(0, 16, &vp);
                commandList.SetConstant(1, (uint32_t)((arm->GetBones().size() - 1) * 2));
                commandList.SetSRV(2, arm->BoneDebugTransforms.OffsetGPU(0));

                commandList.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

                commandList.Draw(1);
            }
        }
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::RenderGUI(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = _currentFrame->_targetHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = _currentFrame->_depthHeap->GetCPUDescriptorHandleForHeapStart();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
    {
        commandList.TransitionBarrier(_currentFrame->_depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        commandList.TransitionBarrier(_gBuffer.GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList.TransitionBarrier(_gBuffer.GetNormalTexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList.SetViewport(_camera.GetViewport());
        commandList.SetRenderTarget(&rtv, &dsv);

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

        GUI::Render(commandList);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}

void DXRenderer::Present(TaskGPU& task)
{
    dx12::CommandList& commandList = *task.GetCommandLists().front();

    PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "Present");
    {
        commandList.TransitionBarrier(*_currentFrame->_swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        commandList.TransitionBarrier(_currentFrame->_targetTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandList.CopyResource(_currentFrame->_targetTexture, *_currentFrame->_swapChainTexture);
        commandList.TransitionBarrier(*_currentFrame->_swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
    }
    PIXEndEvent(commandList.GetDXCommandList().Get());

    commandList.Close();
}
