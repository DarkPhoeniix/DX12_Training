#include "RendererPCH.h"

#include "SSAOBlurPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    using namespace DirectX;

    namespace
    {
        constexpr int kRadius = 4;
        constexpr float kDepthThreshold = 0.2f;
        constexpr float kSharpness = 50.0f;

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
        : RenderPass<SSAOBlurPassData>("SSAO Blur Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _SSAOBlurHorizonralPipeline.Parse("PipelineDescriptions\\SSAOBlurHorizontalPipeline.tech");
        _SSAOBlurVerticalPipeline.Parse("PipelineDescriptions\\SSAOBlurVerticalPipeline.tech");
    }

    void SSAOBlurPass::Setup(rg::RenderPassBuilder& builder)
    {
        dx12::ResourceDescription weightsBufferDesc;
        {
            weightsBufferDesc.SetSize({ (kRadius * 2 + 1) * sizeof(float), 1 });
            weightsBufferDesc.SetStride(sizeof(float));
            weightsBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
		}
		std::vector<float> weightsData(kRadius * 2 + 1);
        const float sigma = 2.0f;
        float sum = 0.0f;
        for (int i = -kRadius; i <= kRadius; ++i)
        {
            float weight = std::exp(-0.5f * (float(i) / sigma) * (float(i) / sigma));

            weightsData[kRadius + i] = weight;
            sum += weight;
        }
        for (int i = -kRadius; i <= kRadius; ++i)
        {
            weightsData[kRadius + i] /= sum;
        }
		_data.WeightsBuffer = builder.CreateResource("ssao_blur_weights", weightsBufferDesc, weightsData.data(), sizeof(float) * weightsData.size());

        _data.Depth = builder.ReadResource("depth_target");
        _data.AOTarget = builder.ReadResource("ao_target");

        dx12::ResourceDescription aoBlurDesc;
        {
            aoBlurDesc.SetSize(_camera->GetViewport().GetSize());
            aoBlurDesc.SetFormat(DXGI_FORMAT_R32_FLOAT);
            aoBlurDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.TempBlurTarget = builder.CreateResource("ao_blur_target", aoBlurDesc);
    }

    void SSAOBlurPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("SSAO Blur pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO Blur");
        {
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

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { depth,      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { aoTarget,   D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { blurTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_SSAOBlurHorizonralPipeline);

            PassConstants passCB =
            {
                .Radius = kRadius,
                .DepthThreshold = kDepthThreshold,
                .Sharpness = kSharpness,
                .WeightsBufferIndex = weightsBufferSRV.Index,
                .DepthTextureIndex = depthHandle.Index,
                .InputTextureIndex = aoTargetSRV.Index,
                .OutputTextureIndex = blurTargetUAV.Index
			};
			commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
			commandList.SetConstants(1, 7, &passCB);

            XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { aoTarget,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { blurTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_SSAOBlurVerticalPipeline);

            commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
            commandList.SetConstants(1, 7, &passCB);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { depth,      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { aoTarget,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
                { blurTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render