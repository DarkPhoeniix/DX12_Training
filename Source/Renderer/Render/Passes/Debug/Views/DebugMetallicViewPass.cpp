#include "RendererPCH.h"

#include "DebugMetallicViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugMetallicViewPass::DebugMetallicViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugMetallicViewPassData>(device, "debug_metallic_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugMetallicViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugMetallicView.tech");
	}

	void DebugMetallicViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugMetallicViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Debug View Pass - Metallic", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.AlbedoMetallic, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugMetallicViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, nullptr);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV) };
			commandList->SetGraphicsCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
			commandList->SetGraphicsConstants(1, 1, &PassConstants);

			commandList->Draw(3);
		}

		commandList->Close();
	}
} // namespace render
