#include "RendererPCH.h"

#include "DebugRoughnessViewPass.h"

#include "CommandList.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugRoughnessViewPass::DebugRoughnessViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugRoughnessViewPassData>("Debug Roughness Pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugRoughnessViewPipeline.Parse("PipelineDescriptions\\DebugRoughnessView.tech");
	}

	void DebugRoughnessViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugRoughnessViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Render Debug Roughness command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 9, "Debug View - Roughness");
		{
			std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle normalRoughnessHandle = context.GetStaticResourceHandle(normalRoughness->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugRoughnessViewPipeline);

			commandList.SetViewport(_camera->GetViewport());
			commandList.SetRenderTarget(&renderTargetHandle.CpuHandle, nullptr);

			commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = normalRoughnessHandle.Index };
			commandList.SetConstants(1, 1, &PassConstants);

			commandList.Draw(3);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
