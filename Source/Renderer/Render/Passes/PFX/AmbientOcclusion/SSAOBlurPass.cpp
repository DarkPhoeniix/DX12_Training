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

	SSAOBlurPass::SSAOBlurPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOBlurPassData>("ssao_blur_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOBlurHorizonralPipeline.Parse("PipelineDescriptions\\SSAOBlurHorizontalPipeline.tech");
		_SSAOBlurVerticalPipeline.Parse("PipelineDescriptions\\SSAOBlurVerticalPipeline.tech");
	}

	void SSAOBlurPass::Setup(rg::RenderPassBuilder& builder)
	{
        int radius = RenderSettings::SSAO().BlurRadius;

		dx12::ResourceDescription weightsBufferDesc;
		{
			weightsBufferDesc.SetSize({ (std::uint32_t)((radius * 2 + 1) * sizeof(float)), 1 });
			weightsBufferDesc.SetStride(sizeof(float));
			weightsBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
		}
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

		dx12::ResourceDescription aoBlurDesc;
		{
			aoBlurDesc.SetSize(_camera->GetViewport().GetSize());
			aoBlurDesc.SetFormat(DXGI_FORMAT_R32_FLOAT);
			aoBlurDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
		}
        builder.DeclareTexture("ao_blur_target", aoBlurDesc);

		_data.Depth = builder.DepthStencilRead("depth_target");
		_data.AOTarget = builder.WriteTexture("ao_target");
        _data.TempBlurTarget = builder.WriteTexture("ao_blur_target");
		_data.WeightsBuffer = builder.ReadBuffer("ssao_blur_weights");
	}

	void SSAOBlurPass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("ssao_blur_pass_cmd_list");

		{
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 3, "SSAO Blur Pass");

			std::shared_ptr<dx12::Resource> weights = context.GetResource(_data.WeightsBuffer);
			std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
			std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);
			std::shared_ptr<dx12::Resource> blurTarget = context.GetResource(_data.TempBlurTarget);

			DescriptorHandle weightsBufferSRV = context.GetStaticResourceHandle(weights->GetAsSRV());
			DescriptorHandle depthHandle = context.GetStaticResourceHandle(depth->GetAsSRV());
			DescriptorHandle aoTargetSRV = context.GetStaticResourceHandle(aoTarget->GetAsSRV());
			DescriptorHandle blurTargetSRV = context.GetStaticResourceHandle(blurTarget->GetAsSRV());
			DescriptorHandle aoTargetUAV = context.GetStaticResourceHandle(aoTarget->GetAsUAV());
			DescriptorHandle blurTargetUAV = context.GetStaticResourceHandle(blurTarget->GetAsUAV());

			context.BindBindlessTable(commandList);
			commandList.SetPipelineState(_SSAOBlurHorizonralPipeline);

			PassConstants passCB =
			{
				.Radius = static_cast<int>(RenderSettings::SSAO().BlurRadius),
				.DepthThreshold = RenderSettings::SSAO().DepthThreshold,
				.Sharpness = RenderSettings::SSAO().Sharpness,
				.WeightsBufferIndex = weightsBufferSRV.Index,
				.DepthTextureIndex = depthHandle.Index,
				.InputTextureIndex = aoTargetSRV.Index,
				.OutputTextureIndex = blurTargetUAV.Index
			};
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 7, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList.Dispatch(xThreadGroups, yThreadGroups);

			context.BindBindlessTable(commandList);
			commandList.SetPipelineState(_SSAOBlurVerticalPipeline);

			passCB.InputTextureIndex = blurTargetUAV.Index;
			passCB.OutputTextureIndex = aoTargetSRV.Index;
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 7, &passCB);

			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList.Close();
	}
} // namespace render