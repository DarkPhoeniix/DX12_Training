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

		struct PassCB
		{
			float MinLogLuminance;
			float OneOverLogLuminanceRange;
			std::uint32_t HDRTextureIndex;
			std::uint32_t LuminanceHistogramBufferIndex;
		};
	} // namespace unnamed

	LuminanceHistogramPass::LuminanceHistogramPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<LuminanceHistogramPassData>("luminance_histogram_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_luminanceHistogramPipeline.Parse("PipelineDescriptions\\BuildLuminanceHistogramPipeline.tech");
	}

	void LuminanceHistogramPass::Setup(rg::RenderPassBuilder& builder)
	{
		dx12::ResourceDescription lumDesc;
		{
			lumDesc.SetSize({ LUM_HISTOGRAM_BINS_NUM * sizeof(std::uint32_t), 1 });
			lumDesc.SetStride(sizeof(std::uint32_t));
			lumDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Unordered);
		}
		builder.DeclareBuffer("luminance_histogram", lumDesc);

		_data.LuminanceHistogram = builder.WriteBuffer("luminance_histogram");
		_data.HDRTarget = builder.ReadTexture("hdr_target");
	}

	void LuminanceHistogramPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("luminance_histogram_pass_cmd_list");

		{
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 5, "Build Luminance Histogram Pass");

			// Copy and setup needed resources

			std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
			std::shared_ptr<dx12::Resource> luminanceHistogram = context.GetResource(_data.LuminanceHistogram);

			DescriptorHandle harTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsSRV());
			DescriptorHandle luminanceHistogramHandle = context.GetStaticResourceHandle(luminanceHistogram->GetAsUAV());

			// Setup pipeline state

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_luminanceHistogramPipeline);

			// Setup root signature components

			PassCB constants =
			{
				.MinLogLuminance = RenderSettings::ToneMapping().MinLogLuminance,
				.OneOverLogLuminanceRange = 1.0f / (RenderSettings::ToneMapping().MaxLogLuminance - RenderSettings::ToneMapping().MinLogLuminance),
				.HDRTextureIndex = static_cast<uint32_t>(harTargetHandle.Index),
				.LuminanceHistogramBufferIndex = static_cast<uint32_t>(luminanceHistogramHandle.Index)
			};
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 4, &constants);

			// Execute

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(LUM_HISTOGRAM_THREADS_NUM));
			std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(LUM_HISTOGRAM_THREADS_NUM));
			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList.Close();
	}
} // namespace render
