#include "RendererPCH.h"

#include "DebugEmissiveViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugEmissiveViewPass::DebugEmissiveViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugEmissiveViewPassData>(device, "debug_emissive_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugEmissiveViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugEmissiveView.tech");
	}

	void DebugEmissiveViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.Emission = builder.ReadTexture("emission_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugEmissiveViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Debug View Pass - Emissive", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.Emission, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugEmissiveViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, nullptr);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = context.GetBindlessIndex(_data.Emission, rhi::ResourceViewType::SRV) };
			commandList->SetGraphicsCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
			commandList->SetGraphicsConstants(1, 1, &PassConstants);

			commandList->Draw(3);
		}

		commandList->Close();
	}
} // namespace render
