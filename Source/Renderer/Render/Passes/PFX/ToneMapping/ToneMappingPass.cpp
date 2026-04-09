#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "Core/RenderSettings.h"
#include "Scene/Entity/Components/Camera.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

namespace render
{
	namespace
	{
		constexpr std::uint32_t LUM_HISTOGRAM_BINS_NUM = 256;
		constexpr std::uint32_t LUM_HISTOGRAM_THREADS_NUM = 16;
		constexpr std::uint32_t TONE_MAPPING_THREADS_NUM = 8;

		struct PassConstants
		{
			float MiddleGrey;
			float White;
			float Gamma;

			std::uint32_t HDRTextureIndex;
			std::uint32_t AverageLuminanceBufferIndex;
			std::uint32_t TargetTextureIndex;
		};
	} // namespace unnamed

	ToneMappingPass::ToneMappingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<ToneMappingPassData>(device, "tone_mapping_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_toneMappingPipeline = _device->CreatePipelineState("PipelineDescriptions\\ToneMappingPipeline.tech");
	}

	void ToneMappingPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::TextureDescription targetDesc =
		{
			.Width = _camera->GetViewport().GetSize().x,
			.Height = _camera->GetViewport().GetSize().y,
			.Format = rhi::Format::R8G8B8A8_UNORM,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowRenderTarget | rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareTexture("render_target", targetDesc);

		_data.Target = builder.WriteTexture("render_target");
		_data.AverageLuminance = builder.ReadBuffer("average_luminance");
		_data.HDRTarget = builder.ReadTexture("hdr_target");
	}

	void ToneMappingPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Tone Mapping Pass", 7);

			// Setup pipeline state

			commandList->SetComputePipelineState(_toneMappingPipeline.get());

			// Setup root signature components

			PassConstants constants =
			{
				.MiddleGrey = RenderSettings::ToneMapping().MiddleGrey,
				.White = RenderSettings::ToneMapping().WhitePoint,
				.Gamma = RenderSettings::ToneMapping().Gamma,

				.HDRTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::SRV),
				.AverageLuminanceBufferIndex = context.GetBindlessIndex(_data.AverageLuminance, rhi::ResourceViewType::SRV),
				.TargetTextureIndex = context.GetBindlessIndex(_data.Target, rhi::ResourceViewType::UAV),
			};
			commandList->SetComputeConstants(1, 6, &constants);

			// Execute

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(TONE_MAPPING_THREADS_NUM));
			std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(TONE_MAPPING_THREADS_NUM));

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render
