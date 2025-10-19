#include "RendererPCH.h"

#include "SSAOApplyPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
	struct PassConstants
	{
		std::uint32_t AOTargetIndex;
		std::uint32_t HDRTargetIndex;
	};
}

namespace render
{
	using namespace DirectX;

	SSAOApplyPass::SSAOApplyPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOApplyPassData>("ssao_apply_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOPipeline.Parse("PipelineDescriptions\\SSAOApplyPipeline.tech");
	}

	void SSAOApplyPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.AOTarget = builder.ReadTexture("ao_target");
		_data.HDRTarget = builder.WriteTexture("hdr_target");
	}

	void SSAOApplyPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("ssao_apply_pass_cmd_list");

		{
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 4, "SSAO Apply Pass");

			std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);
			std::shared_ptr<dx12::Resource> hdtTarget = context.GetResource(_data.HDRTarget);

			DescriptorHandle aoTargetHandle = context.GetStaticResourceHandle(aoTarget->GetAsSRV());
			DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdtTarget->GetAsUAV());

			context.BindBindlessTable(commandList);
			commandList.SetPipelineState(_SSAOPipeline);

			PassConstants passCB =
			{
				.AOTargetIndex = aoTargetHandle.Index,
				.HDRTargetIndex = hdrTargetHandle.Index
			};
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 2, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList.Close();
	}
} // namespace render