#include "RendererPCH.h"

#include "DebugEmissiveViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugEmissiveViewPass::DebugEmissiveViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugEmissiveViewPassData>("debug_emissive_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugEmissiveViewPipeline.Parse("PipelineDescriptions\\DebugEmissiveView.tech");
	}

	void DebugEmissiveViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Emission = builder.ReadTexture("emission_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugEmissiveViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("debug_emissive_cmd_list");

		{
			PIXScopedEvent(commandList.GetDXCommandList().Get(), 9, "Debug View Pass - Emissive");

			std::shared_ptr<dx12::Resource> emission = context.GetResource(_data.Emission);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle emissionHandle = context.GetStaticResourceHandle(emission->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugEmissiveViewPipeline);

			commandList.SetViewport(_camera->GetViewport());
			commandList.SetRenderTarget(&renderTargetHandle.CpuHandle, nullptr);

			commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = emissionHandle.Index };
			commandList.SetConstants(1, 1, &PassConstants);

			commandList.Draw(3);
		}

		commandList.Close();
	}
} // namespace render
