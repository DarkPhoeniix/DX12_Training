#include "RendererPCH.h"

#include "DebugSSAOViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugSSAOViewPass::DebugSSAOViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugSSAOViewPassData>(device, "debug_ssao_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugSSAOViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugSSAOView.tech");
	}

	void DebugSSAOViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.SSAOTexture = builder.ReadTexture("ao_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugSSAOViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Debug View Pass - SSAO", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.SSAOTexture, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugSSAOViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, nullptr);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = context.GetBindlessIndex(_data.SSAOTexture, rhi::ResourceViewType::SRV) };
			commandList->SetGraphicsCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
			commandList->SetGraphicsConstants(1, 1, &PassConstants);

			commandList->Draw(3);
		}

		commandList->Close();
	}
} // namespace render
