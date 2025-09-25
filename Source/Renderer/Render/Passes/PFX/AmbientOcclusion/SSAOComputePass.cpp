#include "RendererPCH.h"

#include "SSAOComputePass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

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
		constexpr float kRadius = 2.5f;
		constexpr float kBias = 0.025f;

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

	SSAOComputePass::SSAOComputePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
		: RenderPass<SSAOComputePassData>("SSAO Pass", rg::RenderPassType::Compute)
		, _scene(scene)
		, _camera(camera)
	{
		_SSAOPipeline.Parse("PipelineDescriptions\\SSAOComputePipeline.tech");
	}

	void SSAOComputePass::Setup(rg::RenderPassBuilder& builder)
	{
		dx12::ResourceDescription noiseDesc;
		{
			noiseDesc.SetSize({ 64 * sizeof(XMVECTOR), 1 });
			noiseDesc.SetStride(sizeof(XMVECTOR));
			noiseDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
		}
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

		dx12::ResourceDescription kernelsDesc;
		{
			kernelsDesc.SetSize({ kKernelSize * sizeof(XMVECTOR), 1 });
			kernelsDesc.SetStride(sizeof(XMVECTOR));
			kernelsDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
		}
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

		dx12::ResourceDescription aoDesc;
		{
			aoDesc.SetSize(_camera->GetViewport().GetSize());
			aoDesc.SetFormat(DXGI_FORMAT_R32_FLOAT);
			aoDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
		}
		builder.DeclareTexture("ao_target", aoDesc);

        _data.Noise = builder.ReadBuffer("ssao_noise");
        _data.Kernels = builder.ReadBuffer("ssao_kernels");
		_data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
		_data.Depth = builder.DepthStencilRead("depth_target");
        _data.AOTarget = builder.WriteTexture("ao_target");
	}

	void SSAOComputePass::Execute(rg::RenderContext& context, TaskGPU& task)
	{
		dx12::CommandList& commandList = *task.GetCommandLists().front();
		commandList.SetName("SSAO pass command list");

		PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO");
		{
			std::shared_ptr<dx12::Resource> noise = context.GetResource(_data.Noise);
			std::shared_ptr<dx12::Resource> kernels = context.GetResource(_data.Kernels);
			std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
			std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
			std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);

			DescriptorHandle noiseHandle = context.GetStaticResourceHandle(noise->GetAsSRV());
			DescriptorHandle kernelsHandle = context.GetStaticResourceHandle(kernels->GetAsSRV());
			DescriptorHandle normalSpecularHandle = context.GetStaticResourceHandle(normalRoughness->GetAsSRV());
			DescriptorHandle depthHandle = context.GetStaticResourceHandle(depth->GetAsSRV());
			DescriptorHandle aoTargetHandle = context.GetStaticResourceHandle(aoTarget->GetAsUAV());

			context.BindBindlessTable(commandList);
			commandList.SetPipelineState(_SSAOPipeline);

			PassConstants passCB =
			{
				.KernelSize = kKernelSize,
				.Radius = kRadius,
				.Bias = kBias,
				.KernelBufferIndex = kernelsHandle.Index,
				.NoiseBufferIndex = noiseHandle.Index,
				.DepthTextureIndex = depthHandle.Index,
				.NormalMapTextureIndex = normalSpecularHandle.Index,
				.AmbientOcclusionTextureIndex = aoTargetHandle.Index
			};
			commandList.SetCBV(0, context.GetFrame()->GetBuffer()->OffsetGPU());
			commandList.SetConstants(1, 8, &passCB);

			XMUINT2 viewportSize = _camera->GetViewport().GetSize();
			int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
			int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

			commandList.Dispatch(xThreadGroups, yThreadGroups);
		}
		PIXEndEvent(commandList.GetDXCommandList().Get());

		commandList.Close();
	}
} // namespace render
