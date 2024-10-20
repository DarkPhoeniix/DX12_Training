#include "Pch.h"

#include "DXRenderer.h"

#include "DXObjects/GraphicsCommandList.h"
#include "GUI/GUI.h"
#include "Render/TaskGPU.h"
#include "Utility/DebugInfo.h"

//#include "Scene/Nodes/Objects/StaticObject.h"

using namespace DirectX;
using namespace Core;

namespace
{
    constexpr float MOVE_SPEED = 200.0f;
} // namespace unnamed

ForwardShadingRenderer::ForwardShadingRenderer(HWND windowHandle)
    : IRenderer(windowHandle)
    , _isCameraMoving(false)
    , _deltaTime(0.0f)
{   }

ForwardShadingRenderer::~ForwardShadingRenderer()
{   }

bool ForwardShadingRenderer::LoadContent(TaskGPU* loadTask)
{
    _renderPipeline.Parse("PipelineDescriptions\\TriangleRenderPipeline.tech");
    _AABBpipeline.Parse("PipelineDescriptions\\AABBRenderPipeline.tech");

    //{
    //    SceneLayer::StaticObject o;
    //    o.SetName("Test");
    //}

    // Camera Setup
    {
        XMVECTOR pos = XMVectorSet(-10.0f, 15.0f, -30.0f, 1.0f);
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

void ForwardShadingRenderer::UnloadContent()
{
    _contentLoaded = false;
}

void ForwardShadingRenderer::OnUpdate(Events::UpdateEvent& updateEvent)
{
    DebugInfo::Update(updateEvent);

    _deltaTime = updateEvent.elapsedTime;
}

void ForwardShadingRenderer::OnRender(Events::RenderEvent& renderEvent, Frame& frame)
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

        {
            commandList.TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

            FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

            commandList.ClearRTV(rtv, clearColor);
            commandList.ClearDSV(dsv, D3D12_CLEAR_FLAG_DEPTH);
        }

        commandList.Close();
    }

    // Execute the TriangleRender shader
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_renderPipeline);
        task->SetName("render");
        task->AddDependency("clean");

        Core::GraphicsCommandList& commandList = *task->GetCommandLists().front();

        {
#if defined(_DEBUG)
            DebugInfo::StartStatCollecting(commandList);
#endif

            commandList.SetPipelineState(_renderPipeline);
            commandList.SetGraphicsRootSignature(_renderPipeline);

            commandList.SetViewport(_camera.GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            _scene.Draw(commandList);

#if defined(_DEBUG)
            commandList.SetPipelineState(_AABBpipeline);
            commandList.SetGraphicsRootSignature(_AABBpipeline);
            commandList.SetConstants(1, sizeof(XMMATRIX) / 4, &_camera.ViewProjection());

            _scene.DrawAABB(commandList);

            DebugInfo::EndStatCollecting(commandList);
#endif
        }

        commandList.Close();
    }

    //GUI
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        task->AddDependency("render");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();

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

        commandList->Close();
    }

    // Present
    {
        TaskGPU* task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("present");
        task->AddDependency("gui");

        Core::GraphicsCommandList* commandList = task->GetCommandLists().front();

        {
            commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
            commandList->TransitionBarrier(frame._targetTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
            commandList->CopyResource(frame._targetTexture, frame._swapChainTexture);
            commandList->TransitionBarrier(frame._swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
        }

        commandList->Close();
    }
}

void ForwardShadingRenderer::OnKeyPressed(Events::KeyEvent& e)
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

void ForwardShadingRenderer::OnMouseMoved(Events::MouseMoveEvent& e)
{
    if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
    {
        _camera.Update(e.relativeX, e.relativeY);
    }
}

void ForwardShadingRenderer::OnMouseButtonPressed(Events::MouseButtonEvent& e)
{
    if (e.rightButton)
    {
        _isCameraMoving = true;
    }
}

void ForwardShadingRenderer::OnMouseButtonReleased(Events::MouseButtonEvent& e)
{
    if (!e.rightButton)
    {
        _isCameraMoving = false;
    }
}
