#include "RendererPCH.h"

#include "DebugRoughnessViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugRoughnessViewPass::DebugRoughnessViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugRoughnessViewPassData>(device, "debug_roughness_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugRoughnessViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugRoughnessView.tech");
	}

	void DebugRoughnessViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugRoughnessViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Debug View Pass - Roughness", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.NormalRoughness, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugRoughnessViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport().GetNativeViewport(), _camera->GetViewport().GetScissorRectangle());
			commandList->SetRenderTarget(&targetHandle, nullptr);

			commandList->SetPrimitiveTopology(rhi::PrimitiveTopology::TriangleList);

			struct
			{
				std::uint32_t SourceTextureIndex;
			} PassConstants = { .SourceTextureIndex = context.GetBindlessIndex(_data.NormalRoughness, rhi::ResourceViewType::SRV) };
			commandList->SetGraphicsCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
			commandList->SetGraphicsConstants(1, 1, &PassConstants);

			commandList->Draw(3);
		}

		commandList->Close();
	}
} // namespace render
