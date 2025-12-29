#include "EditorPCH.h"

#include "GUIPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace render
{
    GUIPass::GUIPass(std::shared_ptr<gui::Editor> editor)
        : rg::RenderPass<GUIPassData>("GUI Pass", rg::RenderPassType::Graphics)
        , _editor(editor)
    {
    }

    void GUIPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.RenderTarget("render_target");
        _data.Depth = builder.DepthStencilWrite("depth_target");
    }

    void GUIPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        {
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 5, "GUI");

            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            DescriptorHandle rtv = context.GetStaticResourceHandle(target->GetAsRTV());
            DescriptorHandle dsv = context.GetStaticResourceHandle(depth->GetAsDSV());

            commandList.SetViewport(_editor->GetViewport()->GetDXViewport(), _editor->GetViewport()->GetScissorRectangle());
            commandList.SetRenderTarget(&rtv.CpuHandle, &dsv.CpuHandle);

            _editor->Update();
            _editor->Render(commandList);
        }

        commandList.Close();
    }
} // namespace render
