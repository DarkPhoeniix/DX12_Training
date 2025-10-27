#include "RendererPCH.h"

#include "DebugNormalViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugNormalViewPass::DebugNormalViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugNormalViewPassData>("debug_normal_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugNormalViewPipeline.Parse("PipelineDescriptions\\DebugNormalView.tech");
	}

	void DebugNormalViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugNormalViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("debug_normal_cmd_list");

		{
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 9, "Debug View Pass - Normal");

			std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle normalRoughnessHandle = context.GetStaticResourceHandle(normalRoughness->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugNormalViewPipeline);

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

		commandList.Close();
	}
} // namespace render
