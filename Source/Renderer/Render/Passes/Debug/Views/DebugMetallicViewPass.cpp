#include "RendererPCH.h"

#include "DebugMetallicViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugMetallicViewPass::DebugMetallicViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugMetallicViewPassData>("debug_metallic_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugMetallicViewPipeline.Parse("PipelineDescriptions\\DebugMetallicView.tech");
	}

	void DebugMetallicViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugMetallicViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("debug_metallic_cmd_list");

		{
			PIXScopedEvent(commandList.GetDXCommandList().Get(), 9, "Debug View Pass - Metallic");

			std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle albedoMetallicHandle = context.GetStaticResourceHandle(albedoMetallic->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugMetallicViewPipeline);

			commandList.SetViewport(_camera->GetViewport());
			commandList.SetRenderTarget(&renderTargetHandle.CpuHandle, nullptr);

			commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = albedoMetallicHandle.Index };
			commandList.SetConstants(1, 1, &PassConstants);

			commandList.Draw(3);
		}

		commandList.Close();
	}
} // namespace render
