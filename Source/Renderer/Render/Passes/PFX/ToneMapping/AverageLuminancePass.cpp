#include "RendererPCH.h"

#include "AverageLuminancePass.h"

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

		constexpr float MIN_LOG_LUM = -10.0f;
		constexpr float MAX_LOG_LUM = 4.0f;
		constexpr float LOG_LUM_RANGE = (MAX_LOG_LUM - MIN_LOG_LUM);
		constexpr float RCP_LOG_LUM_RANGE = 1.0f / LOG_LUM_RANGE;

		struct PassConstants
		{
			std::uint32_t PixelCount;
			float MinLogLuminance;
			float LogLuminanceRange;

			std::uint32_t LuminanceHistogramIndex;
			std::uint32_t AverageLuminanceBufferIndex;
		};
	} // namespace unnamed

	AverageLuminancePass::AverageLuminancePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<AverageLuminancePassData>(device, "average_luminance_pass", rg::RenderPassType::Graphics)
		, _scene(scene)
		, _camera(camera)
	{
		_averageLuminancePipeline = _device->CreatePipelineState("PipelineDescriptions\\AverageLuminancePipeline.tech");
	}

	void AverageLuminancePass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::BufferDescription lumDesc =
		{
			.Size = sizeof(float),
			.Stride = sizeof(float),
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareBuffer("average_luminance", lumDesc);

		_data.AverageLuminance = builder.WriteBuffer("average_luminance");
		_data.LuminanceHistogram = builder.WriteBuffer("luminance_histogram");
	}

	void AverageLuminancePass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "Compute Average Luminance Pass", 5);

			// Setup pipeline state

			commandList->SetComputePipelineState(_averageLuminancePipeline.get());

			// Setup root signature components

			DirectX::XMUINT2 viewportSize = _camera->GetSize();
			std::uint32_t size = viewportSize.x * viewportSize.y;

			PassConstants passConstants =
			{
				.PixelCount = size,
				.MinLogLuminance = MIN_LOG_LUM,
				.LogLuminanceRange = LOG_LUM_RANGE,

				.LuminanceHistogramIndex = context.GetBindlessIndex(_data.LuminanceHistogram, rhi::ResourceViewType::UAV),
				.AverageLuminanceBufferIndex = context.GetBindlessIndex(_data.AverageLuminance, rhi::ResourceViewType::UAV),
			};
			commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
			commandList->SetComputeConstants(1, 5, &passConstants);

			// Execute

			commandList->Dispatch();
		}

		commandList->Close();
	}
} // namespace render
