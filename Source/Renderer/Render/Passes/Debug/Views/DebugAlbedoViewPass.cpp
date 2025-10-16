#include "RendererPCH.h"

#include "DebugAlbedoViewPass.h"

#include "CommandList.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugAlbedoViewPass::DebugAlbedoViewPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugAlbedoViewPassData>("Debug Albedo Pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugAlbedoViewPipeline.Parse("PipelineDescriptions\\DebugAlbedoView.tech");
	}

	void DebugAlbedoViewPass::Setup(rg::RenderPassBuilder& builder)
	{
        _data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugAlbedoViewPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Render Debug Albedo command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 9, "Debug View - Albedo");
		{
            std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

            DescriptorHandle albedoMetallicHandle = context.GetStaticResourceHandle(albedoMetallic->GetAsSRV());
			DescriptorHandle renderTargetHandle = context.GetStaticResourceHandle(target->GetAsRTV());

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_debugAlbedoViewPipeline);

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
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
