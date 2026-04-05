#include "RendererPCH.h"

#include "LuminanceHistogramPass.h"

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
			float MinLogLuminance;
			float OneOverLogLuminanceRange;
			std::uint32_t HDRTextureIndex;
			std::uint32_t LuminanceHistogramBufferIndex;
		};
	} // namespace unnamed

	LuminanceHistogramPass::LuminanceHistogramPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<LuminanceHistogramPassData>(device, "luminance_histogram_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_luminanceHistogramPipeline = _device->CreatePipelineState("PipelineDescriptions\\BuildLuminanceHistogramPipeline.tech");
	}

	void LuminanceHistogramPass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::BufferDescription lumDesc =
		{
			.Size = LUM_HISTOGRAM_BINS_NUM * sizeof(std::uint32_t),
			.Stride = sizeof(std::uint32_t),
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareBuffer("luminance_histogram", lumDesc);

		_data.LuminanceHistogram = builder.WriteBuffer("luminance_histogram");
		_data.HDRTarget = builder.ReadTexture("hdr_target");
	}

	void LuminanceHistogramPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Build Luminance Histogram Pass", 5);

			// Setup pipeline state

			commandList->SetComputePipelineState(_luminanceHistogramPipeline.get());

			// Setup root signature components

			PassConstants constants =
			{
				.MinLogLuminance = RenderSettings::ToneMapping().MinLogLuminance,
				.OneOverLogLuminanceRange = 1.0f / (RenderSettings::ToneMapping().MaxLogLuminance - RenderSettings::ToneMapping().MinLogLuminance),
				.HDRTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::SRV),
				.LuminanceHistogramBufferIndex = context.GetBindlessIndex(_data.LuminanceHistogram, rhi::ResourceViewType::UAV),
			};
			commandList->SetComputeConstants(1, 4, &constants);

			// Execute

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
			std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render
