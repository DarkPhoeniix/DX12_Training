#include "RendererPCH.h"

#include "AverageLuminancePass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

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

		struct PassCB
		{
			std::uint32_t PixelCount;
			float MinLogLuminance;
			float LogLuminanceRange;

			std::uint32_t PrevLuminanceIndex;
			std::uint32_t LuminanceHistogramIndex;
			std::uint32_t OutputLuminanceIndex;
		};
	} // namespace unnamed

	AverageLuminancePass::AverageLuminancePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<AverageLuminancePassData>("Average Luminance Pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_averageLuminancePipeline.Parse("PipelineDescriptions\\AverageLuminancePipeline.tech");
	}

	void AverageLuminancePass::Setup(rg::RenderPassBuilder& builder)
	{
		_data.LuminanceHistogram = builder.ReadResource("luminance_histogram");

		dx12::ResourceDescription lumDesc;
		{
			lumDesc.SetSize({ sizeof(float), 1 });
			lumDesc.SetStride(sizeof(float));
			lumDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
		}
		_data.PrevAverageLuminance = builder.CreateResource("prev_average_luminance", lumDesc);
		_data.AverageLuminance = builder.CreateResource("average_luminance", lumDesc);
	}

	void AverageLuminancePass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Luminance histogram pass command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Average luminance");
		{
			// Copy and setup needed resources

			std::shared_ptr<dx12::Resource> luminanceHistogram = context.GetResource(_data.LuminanceHistogram);
			std::shared_ptr<dx12::Resource> prevAverageLuminance = context.GetResource(_data.PrevAverageLuminance);
			std::shared_ptr<dx12::Resource> averageLuminance = context.GetResource(_data.AverageLuminance);

			DescriptorHandle prevAverageLuminanceHandle = context.GetStaticResourceHandle(prevAverageLuminance->GetAsSRV());
			DescriptorHandle luminanceHistogramHandle = context.GetStaticResourceHandle(luminanceHistogram->GetAsUAV());
			DescriptorHandle averageLuminanceHandle = context.GetStaticResourceHandle(averageLuminance->GetAsUAV());

			std::vector<dx12::ResourceBarrier> barriers =
			{
				{ prevAverageLuminance,   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE},
				{ luminanceHistogram,     D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
				{ averageLuminance,       D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
			};
			commandList.TransitionBarriers(barriers);

			// Setup pipeline state

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_averageLuminancePipeline);

			// Setup root signature components

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			std::uint32_t size = viewportSize.x * viewportSize.y;

			PassCB passConstants =
			{
				.PixelCount = size,
				.MinLogLuminance = MIN_LOG_LUM,
				.LogLuminanceRange = LOG_LUM_RANGE,

				.PrevLuminanceIndex = static_cast<std::uint32_t>(prevAverageLuminanceHandle.Index),
				.LuminanceHistogramIndex = static_cast<std::uint32_t>(luminanceHistogramHandle.Index),
				.OutputLuminanceIndex = static_cast<std::uint32_t>(averageLuminanceHandle.Index)
			};
			commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
			commandList.SetConstants(1, 7, &passConstants);

			// Execute

			std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
			std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
			commandList.Dispatch();

			barriers =
			{
				{ prevAverageLuminance,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
				{ luminanceHistogram,     D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
				{ averageLuminance,       D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
			};
			commandList.TransitionBarriers(barriers);

			barriers =
			{
				{ prevAverageLuminance, D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_COPY_DEST},
				{ averageLuminance,     D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_COPY_SOURCE }
			};
			commandList.TransitionBarriers(barriers);

			commandList.CopyResource(*averageLuminance, *prevAverageLuminance);

			barriers =
			{
				{ prevAverageLuminance, D3D12_RESOURCE_STATE_COPY_DEST,     D3D12_RESOURCE_STATE_COMMON},
				{ averageLuminance,     D3D12_RESOURCE_STATE_COPY_SOURCE,   D3D12_RESOURCE_STATE_COMMON }
			};
			commandList.TransitionBarriers(barriers);

		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
