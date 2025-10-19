#include "RendererPCH.h"

#include "DebugSSAOViewPass.h"

#include "CommandList.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugSSAOViewPass::DebugSSAOViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugSSAOViewPassData>("Debug SSAO Pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugSSAOViewPipeline.Parse("PipelineDescriptions\\DebugSSAOView.tech");
	}

	void DebugSSAOViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.SSAOTexture = builder.ReadTexture("ao_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugSSAOViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Render Debug Albedo command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 9, "Debug View - SSAO");
		{
			std::shared_ptr<dx12::Resource> ssaoTexture = context.GetResource(_data.SSAOTexture);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle ssaoTextureHandle = context.GetStaticResourceHandle(ssaoTexture->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugSSAOViewPipeline);

			commandList.SetViewport(_camera->GetViewport());
			commandList.SetRenderTarget(&renderTargetHandle.CpuHandle, nullptr);

			commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = ssaoTextureHandle.Index };
			commandList.SetConstants(1, 1, &PassConstants);

			commandList.Draw(3);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
