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
        _data.Target = builder.WriteResource("render_target");
        _data.Depth = builder.ReadResource("depth_target");
    }

    void GUIPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render GUI command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "GUI");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            DescriptorHandle rtv = context.GetStaticResourceHandle(target->GetAsRTV());
            DescriptorHandle dsv = context.GetStaticResourceHandle(depth->GetAsDSV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { target, D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_RENDER_TARGET },
                { depth,  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_DEPTH_WRITE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetViewport(*_editor->GetViewport());
            commandList.SetRenderTarget(&rtv.CpuHandle, &dsv.CpuHandle);

            _editor->Update();
            _editor->Render(commandList);

            barriers =
            {
                { target, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON },
                { depth,  D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
