#include "RendererPCH.h"

#include "DebugAlbedoViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugAlbedoViewPass::DebugAlbedoViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugAlbedoViewPassData>(device, "debug_albedo_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugAlbedoViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugAlbedoView.tech");
	}

	void DebugAlbedoViewPass::Setup(rg::RenderPassBuilder& builder)
	{
        _data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugAlbedoViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Debug View Pass - Albedo", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.AlbedoMetallic, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugAlbedoViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport().GetDXViewport(), _camera->GetViewport().GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, nullptr);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

			struct
			{
                std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV) };
            commandList->SetGraphicsConstants(1, 1, &PassConstants);

			commandList->Draw(3);
		}

		commandList->Close();
	}
} // namespace render
