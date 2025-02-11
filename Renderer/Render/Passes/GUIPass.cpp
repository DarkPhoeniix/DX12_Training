#include "RendererPCH.h"

#include "GUIPass.h"

#include "Editor.h"
#include "Widgets/SceneTreeWidget.h"
#include "Scene/Entity/Components/Camera.h"
#include "Utility/DebugInfo.h"

namespace render
{
    void GUIPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "GUIPass";

        _sceneWidget = std::make_shared<gui::SceneTreeWidget>(*_scene);
        _sceneWidget->Init();
    }

    void GUIPass::Destroy()
    {
        IRenderPass::Destroy();

        _sceneWidget->Destroy();
    }

    void GUIPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        dx12::ResourceTable& frameTable = _frame->GetResourceTable();
        dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = frameTable.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
        {
            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            _sceneWidget->Update();

            //if (ImGui::Begin("Debug Info"), true, ImGuiWindowFlags_AlwaysAutoResize)
            //{
            //    ImGui::SetWindowPos({ 0, 0 });
            //    ImGui::SetWindowSize({ 0, 0 });
            //
            //    ImGui::Text("FPS: %i (%.03f ms)", DebugInfo::GetFPS(), DebugInfo::GetMsPerFrame());
            //
            //    if (ImGui::CollapsingHeader("Pipeline statistics"))
            //    {
            //        D3D12_QUERY_DATA_PIPELINE_STATISTICS stats = DebugInfo::GetPipelineStatisctics();
            //        ImGui::Text(std::string("Primitives: " + std::to_string(stats.IAPrimitives)).c_str());
            //        ImGui::Text(std::string("VS invocs: " + std::to_string(stats.VSInvocations)).c_str());
            //        ImGui::Text(std::string("GS invocs: " + std::to_string(stats.GSInvocations)).c_str());
            //        ImGui::Text(std::string("PS invocs: " + std::to_string(stats.PSInvocations)).c_str());
            //    }
            //
            //    if (ImGui::CollapsingHeader("inputs"))
            //    {
            //        ImGuiIO& io = ImGui::GetIO();
            //        if (ImGui::IsMousePosValid())
            //        {
            //            ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            //        }
            //        else
            //        {
            //            ImGui::Text("Mouse pos: <INVALID>");
            //        }
            //        ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
            //    }
            //}
            //ImGui::End();

            gui::Editor::Render(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
