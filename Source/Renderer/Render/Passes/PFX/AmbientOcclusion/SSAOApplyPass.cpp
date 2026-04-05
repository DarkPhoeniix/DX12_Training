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

	SSAOApplyPass::SSAOApplyPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOApplyPassData>(device, "ssao_apply_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOPipeline = _device->CreatePipelineState("PipelineDescriptions\\SSAOApplyPipeline.tech");
	}

	void SSAOApplyPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.AOTarget = builder.ReadTexture("ao_target");
		_data.HDRTarget = builder.WriteTexture("hdr_target");
	}

	void SSAOApplyPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "SSAO Apply Pass", 4);

			commandList->SetComputePipelineState(_SSAOPipeline.get());

			PassConstants passCB =
			{
				.AOTargetIndex = context.GetBindlessIndex(_data.AOTarget, rhi::ResourceViewType::SRV),
				.HDRTargetIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::UAV)
			};
			commandList->SetComputeConstants(1, 2, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render