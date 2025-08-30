#include "RendererPCH.h"

#include "SSAOApplyPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

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
		: RenderPass<SSAOApplyPassData>("SSAO Apply Pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOPipeline.Parse("PipelineDescriptions\\SSAOApplyPipeline.tech");
	}

	void SSAOApplyPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.AOTarget = builder.ReadResource("ao_target");
		_data.HDRTarget = builder.WriteResource("hdr_target");
	}

	void SSAOApplyPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("SSAO apply pass command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO Apply");
		{
			std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);
			std::shared_ptr<dx12::Resource> hdtTarget = context.GetResource(_data.HDRTarget);

			DescriptorHandle aoTargetHandle = context.GetStaticResourceHandle(aoTarget->GetAsSRV());
			DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdtTarget->GetAsUAV());

			std::vector<dx12::ResourceBarrier> barriers =
			{
				{ aoTarget,   D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
				{ hdtTarget,  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
			};
			commandList.TransitionBarriers(barriers);

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

			barriers =
			{
				{ aoTarget,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
				{ hdtTarget,  D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
			};
			commandList.TransitionBarriers(barriers);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render