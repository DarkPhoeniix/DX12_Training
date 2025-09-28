#include "RendererPCH.h"

#include "ToneMappingPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

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
			float MiddleGrey;
			float White;
			float Gamma;

			std::uint32_t HDRTextureIndex;
			std::uint32_t AverageLuminanceBufferIndex;
			std::uint32_t TargetTextureIndex;
		};
	} // namespace unnamed

	ToneMappingPass::ToneMappingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<ToneMappingPassData>("Tone Mapping Pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_toneMappingPipeline.Parse("PipelineDescriptions\\ToneMappingPipeline.tech");
	}

	void ToneMappingPass::Setup(rg::RenderPassBuilder& builder)
	{
		dx12::ResourceDescription targetDesc;
		{
			D3D12_CLEAR_VALUE clearValue;
			clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			clearValue.Color[0] = 0.0f;
			clearValue.Color[1] = 0.0f;
			clearValue.Color[2] = 0.0f;
			clearValue.Color[3] = 0.0f;

			targetDesc.SetSize(_camera->GetViewport().GetSize());
			targetDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
			targetDesc.SetClearValue(clearValue);
			targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget | dx12::ResourceType::Unordered);
		}
		builder.DeclareTexture("render_target", targetDesc);

		_data.Target = builder.WriteTexture("render_target");
		_data.AverageLuminance = builder.ReadBuffer("average_luminance");
		_data.HDRTarget = builder.ReadTexture("hdr_target");
	}

	void ToneMappingPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("Tone mapping command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 7, "Tone Mapping");
		{
			// Copy and setup needed resources

			std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
			std::shared_ptr<dx12::Resource> avgLuminance = context.GetResource(_data.AverageLuminance);
			std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);

			DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsSRV());
			DescriptorHandle avgLuminanceHandle = context.GetStaticResourceHandle(avgLuminance->GetAsSRV());
			DescriptorHandle targetHandle = context.GetStaticResourceHandle(target->GetAsUAV());

			// Setup pipeline state

			context.BindBindlessTable(commandList);

			commandList.SetPipelineState(_toneMappingPipeline);

			// Setup root signature components

			PassCB constants =
			{
				.MiddleGrey = RenderSettings::ToneMapping().MiddleGrey,
				.White = RenderSettings::ToneMapping().WhitePoint,
				.Gamma = RenderSettings::ToneMapping().Gamma,

				.HDRTextureIndex = hdrTargetHandle.Index,
				.AverageLuminanceBufferIndex = avgLuminanceHandle.Index,
				.TargetTextureIndex = targetHandle.Index
			};
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 6, &constants);

			// Execute

			DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(viewportSize.x / float(TONE_MAPPING_THREADS_NUM));
			std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(viewportSize.y / float(TONE_MAPPING_THREADS_NUM));

			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
