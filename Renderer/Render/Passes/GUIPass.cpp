#include "RendererPCH.h"

#include "GUIPass.h"

#include "Editor.h"
#include "Scene/Entity/Components/Camera.h"

namespace render
{
    void GUIPass::Initialize()
    {
        IRenderPass::Initialize();

        _name = "GUIPass";

        gui::Editor::SetScene(_scene);
    }

    void GUIPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void GUIPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
        task->SetName("gui");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
        {
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();
            dx12::ResourceTable& gBufferTable = _gBuffer->GetResourceTable();

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = frameTable.GetResourceCPUHandle(&_frame->GetTargetTexture(), dx12::ResourceViewType::RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = gBufferTable.GetResourceCPUHandle(&_gBuffer->GetDepthTexture(), dx12::ResourceViewType::DSV);

            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            gui::Editor::Update();
            gui::Editor::Render(commandList);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
