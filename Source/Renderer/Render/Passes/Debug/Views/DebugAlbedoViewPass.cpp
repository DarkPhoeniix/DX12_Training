#include "RendererPCH.h"

#include "DebugAlbedoViewPass.h"

#include "Core/DescriptorHeapManager.h"

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
		// TODO: remove it later
		rhi::TextureDescription targetDesc =
		{
			.Width = _camera->GetViewport().GetSize().x,
			.Height = _camera->GetViewport().GetSize().y,
			.Format = rhi::Format::R8G8B8A8_UNORM,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowRenderTarget | rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareTexture("render_target", targetDesc);

		_data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
		_data.Target = builder.RenderTarget("render_target");
	}

	void DebugAlbedoViewPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
			GPU_SCOPED_EVENT(commandList, "Debug View Pass - Albedo", 9);

			commandList->SetDescriptorHeaps(DescriptorHeapManager::Get().GetShaderResourcesDescriptorHeap()); // TODO: temp workaround

			rhi::CPUDescriptor targetHandle = context.GetDescriptor(_data.Target, rhi::ResourceViewType::RTV);

			commandList->SetGraphicsPipelineState(_debugAlbedoViewPipeline.get());

			commandList->SetViewport(_camera->GetViewport().GetDXViewport(), _camera->GetViewport().GetScissorRectangle());
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
