#include "RendererPCH.h"

#include "SSAOBlurPass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
	using namespace DirectX;

	namespace
	{
		struct PassConstants
		{
			int Radius;
			float DepthThreshold;
			float Sharpness;

			std::uint32_t WeightsBufferIndex;
			std::uint32_t DepthTextureIndex;
			std::uint32_t InputTextureIndex;
			std::uint32_t OutputTextureIndex;
		};
	} // namespace unnamed

	SSAOBlurPass::SSAOBlurPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOBlurPassData>(device, "ssao_blur_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOBlurHorizonralPipeline = _device->CreatePipelineState("PipelineDescriptions\\SSAOBlurHorizontalPipeline.tech");
		_SSAOBlurVerticalPipeline = _device->CreatePipelineState("PipelineDescriptions\\SSAOBlurVerticalPipeline.tech");
	}

	void SSAOBlurPass::Setup(rg::RenderPassBuilder& builder)
	{
        int radius = RenderSettings::SSAO().BlurRadius;

		rhi::BufferDescription weightsBufferDesc =
		{
			.Size = (std::uint32_t)((radius * 2 + 1) * sizeof(float)),
			.Stride = sizeof(float),
			.Usage = rhi::ResourceUsage::Upload
		};
		std::vector<float> weightsData(radius * 2 + 1);
		const float sigma = 2.0f;
		float sum = 0.0f;
		for (int i = -radius; i <= radius; ++i)
		{
			float weight = std::exp(-0.5f * (float(i) / sigma) * (float(i) / sigma));

			weightsData[radius + i] = weight;
			sum += weight;
		}
		for (int i = -radius; i <= radius; ++i)
		{
			weightsData[radius + i] /= sum;
		}
        builder.DeclareBuffer("ssao_blur_weights", weightsBufferDesc, weightsData.data(), sizeof(float) * weightsData.size());

		rhi::TextureDescription aoBlurDesc =
		{
			.Width = _camera->GetViewport().GetSize().x,
			.Height = _camera->GetViewport().GetSize().y,
			.Format = rhi::Format::R32_FLOAT,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
        builder.DeclareTexture("ao_blur_target", aoBlurDesc);

		_data.Depth = builder.DepthStencilRead("depth_target");
		_data.AOTarget = builder.WriteTexture("ao_target");
        _data.TempBlurTarget = builder.WriteTexture("ao_blur_target");
		_data.WeightsBuffer = builder.ReadBuffer("ssao_blur_weights");
	}

	void SSAOBlurPass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "SSAO Blur Pass", 3);

			commandList->SetComputePipelineState(_SSAOBlurHorizonralPipeline.get());

			PassConstants passCB =
			{
				.Radius = static_cast<int>(RenderSettings::SSAO().BlurRadius),
				.DepthThreshold = RenderSettings::SSAO().DepthThreshold,
				.Sharpness = RenderSettings::SSAO().Sharpness,
				.WeightsBufferIndex = context.GetBindlessIndex(_data.WeightsBuffer, rhi::ResourceViewType::SRV),
				.DepthTextureIndex = context.GetBindlessIndex(_data.Depth, rhi::ResourceViewType::SRV),
				.InputTextureIndex = context.GetBindlessIndex(_data.AOTarget, rhi::ResourceViewType::SRV),
				.OutputTextureIndex = context.GetBindlessIndex(_data.TempBlurTarget, rhi::ResourceViewType::UAV)
			};
			commandList->SetComputeConstants(1, 7, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList->Dispatch(xThreadGroups, yThreadGroups);

			commandList->SetComputePipelineState(_SSAOBlurVerticalPipeline.get());

			passCB.InputTextureIndex = context.GetBindlessIndex(_data.TempBlurTarget, rhi::ResourceViewType::SRV);
			passCB.OutputTextureIndex = context.GetBindlessIndex(_data.AOTarget, rhi::ResourceViewType::UAV);
			commandList->SetComputeConstants(1, 7, &passCB);

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render