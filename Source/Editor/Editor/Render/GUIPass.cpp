#include "EditorPCH.h"

#include "GUIPass.h"

#include "Renderer/Core/DescriptorHeapManager.h"
#include "Renderer/Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "RHI/CommandList.h"
#include "RHI/GPUEvent.h"
#include "RHI/ResourceBarrier.h"

namespace render
{
    GUIPass::GUIPass(rhi::Device* device, std::shared_ptr<gui::Editor> editor)
        : rg::RenderPass<GUIPassData>(device, "GUI Pass", rg::RenderPassType::Graphics)
        , _editor(editor)
    {
    }

    void GUIPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.RenderTarget("render_target");
        _data.Depth = builder.DepthStencilWrite("depth_target");
    }

    void GUIPass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList.GetDXCommandList().Get(), "GUI", 5);

            commandList->SetDescriptorHeaps(DescriptorHeapManager::Get().GetShaderResourcesDescriptorHeap()); // TODO: temp workaround

            rhi::CPUDescriptor target = context.GetDescriptor(_data.Target, rhi::ResourceViewType::RTV);
            rhi::CPUDescriptor depth = context.GetDescriptor(_data.Depth, rhi::ResourceViewType::DSV);

            commandList->SetViewport(_editor->GetViewport()->GetDXViewport(), _editor->GetViewport()->GetScissorRectangle());
            commandList->SetRenderTarget(&target, &depth);

            _editor->Update();
            _editor->Render(commandList);
        }

        commandList->Close();
    }
} // namespace render
