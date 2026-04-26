#include "RendererPCH.h"

#include "DebugNormalViewPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	DebugNormalViewPass::DebugNormalViewPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<DebugNormalViewPassData>(device, "debug_normal_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_debugNormalViewPipeline = _device->CreatePipelineState("PipelineDescriptions\\DebugNormalView.tech");
	}

	void DebugNormalViewPass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugNormalViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Debug View Pass - Normal", 9);

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.Target, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugNormalViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport(), _camera->GetScissorRectangle());
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
