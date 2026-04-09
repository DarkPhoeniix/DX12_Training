#include "RendererPCH.h"

#include "SSAOComputePass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include <random>

namespace render
{
	using namespace DirectX;

	namespace
	{
		// TODO: move to parameters
		constexpr std::uint32_t kKernelSize = 16;

		struct PassConstants
		{
			std::uint32_t KernelSize;
			float Radius;
			float Bias;

			std::uint32_t KernelBufferIndex;
			std::uint32_t NoiseBufferIndex;
			std::uint32_t DepthTextureIndex;
			std::uint32_t NormalMapTextureIndex;
			std::uint32_t AmbientOcclusionTextureIndex;
		};
	} // namespace unnamed

	SSAOComputePass::SSAOComputePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOComputePassData>(device, "ssao_compute_pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOPipeline = _device->CreatePipelineState("PipelineDescriptions\\SSAOComputePipeline.tech");
	}

	void SSAOComputePass::Setup(rg::RenderPassBuilder& builder)
	{
		rhi::BufferDescription noiseDesc =
		{
			.Size = 64 * sizeof(XMVECTOR),
			.Stride = sizeof(XMVECTOR),
			.Usage = rhi::ResourceUsage::Upload
		};
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		std::vector<XMVECTOR> noiseData(64);
		for (size_t i = 0; i < noiseData.size(); ++i)
		{
			noiseData[i] = XMVectorSet(
				dis(gen) * 2.0f - 1.0f,
				dis(gen) * 2.0f - 1.0f,
				0.0f,
				0.0f
			);

			noiseData[i] = XMVector3Normalize(noiseData[i]);
		}
        builder.DeclareBuffer("ssao_noise", noiseDesc, noiseData.data(), sizeof(XMVECTOR) * noiseData.size());

		rhi::BufferDescription kernelsDesc
		{
			.Size = kKernelSize * sizeof(XMVECTOR),
			.Stride = sizeof(XMVECTOR),
			.Usage = rhi::ResourceUsage::Upload
		};
		std::vector<XMVECTOR> kernelsData(kKernelSize);
		for (size_t i = 0; i < kKernelSize; ++i)
		{
			kernelsData[i] = XMVectorSet(
				dis(gen) * 2.0f - 1.0f,
				dis(gen) * 2.0f - 1.0f,
				dis(gen),
				0.0f
			);
			kernelsData[i] = XMVector3Normalize(kernelsData[i]);
			float scale = float(i) / float(kKernelSize);
			scale = std::lerp(0.1f, 1.0f, scale * scale);
			kernelsData[i] *= scale;
		}
		builder.DeclareBuffer("ssao_kernels", kernelsDesc, kernelsData.data(), sizeof(XMVECTOR) * kernelsData.size());

		rhi::TextureDescription aoDesc =
		{
			.Width = _camera->GetViewport().GetSize().x,
			.Height = _camera->GetViewport().GetSize().y,
			.Format = rhi::Format::R32_FLOAT,
			.Dimension = rhi::TextureDimension::Texture2D,
			.Flags = rhi::ResourceFlags::AllowUnorderedAccess
		};
		builder.DeclareTexture("ao_target", aoDesc);

		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Depth = builder.DepthStencilRead("depth_target");
        _data.AOTarget = builder.WriteTexture("ao_target");
		_data.Noise = builder.UploadBuffer("ssao_noise");
		_data.Kernels = builder.UploadBuffer("ssao_kernels");
	}

	void SSAOComputePass::Execute(rg::RenderContext& context, rg::ITask* task)
	{
		rhi::CommandList* commandList = task->GetCommandList();

		{
            GPU_SCOPED_EVENT(commandList, "SSAO Compute Pass", 3);

			commandList->SetComputePipelineState(_SSAOPipeline.get());

			PassConstants passCB =
			{
				.KernelSize = kKernelSize,
				.Radius = RenderSettings::SSAO().Radius,
				.Bias = RenderSettings::SSAO().Bias,
				.KernelBufferIndex = context.GetBindlessIndex(_data.Kernels, rhi::ResourceViewType::SRV),
				.NoiseBufferIndex = context.GetBindlessIndex(_data.Noise, rhi::ResourceViewType::SRV),
				.DepthTextureIndex = context.GetBindlessIndex(_data.Depth, rhi::ResourceViewType::SRV),
				.NormalMapTextureIndex = context.GetBindlessIndex(_data.NormalRoughness, rhi::ResourceViewType::SRV),
				.AmbientOcclusionTextureIndex = context.GetBindlessIndex(_data.AOTarget, rhi::ResourceViewType::UAV)
			};
			commandList->SetComputeConstants(1, 8, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList->Dispatch(xThreadGroups, yThreadGroups);
		}

		commandList->Close();
	}
} // namespace render
