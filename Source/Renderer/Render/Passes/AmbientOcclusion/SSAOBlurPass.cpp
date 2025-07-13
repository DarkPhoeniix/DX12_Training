#include "RendererPCH.h"

#include "SSAOBlurPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/RenderSettings.h"
#include "Render/Passes/PassResources.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include <random>

namespace render
{
    using namespace DirectX;

    namespace
    {
        constexpr int kRadius = 4;
        constexpr float kDepthThreshold = 0.2f;
        constexpr float kSharpness = 50.0f;

        struct ConstantsDesc
        {
            int Radius;
            float DepthThreshold;
            float Sharpness;
        };
    } // namespace unnamed

    SSAOBlurPass::SSAOBlurPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<SSAOBlurPassData>("SSAO Blur Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _SSAOBlurHorizonralPipeline.Parse("PipelineDescriptions\\SSAOBlurHorizontalPipeline.tech");
        _SSAOBlurVerticalPipeline.Parse("PipelineDescriptions\\SSAOBlurVerticalPipeline.tech");

        dx12::ResourceDescription weightsDesc;
        {
            weightsDesc.SetSize({ (kRadius * 2 + 1) * sizeof(float), 1});
            weightsDesc.SetStride(sizeof(float));
            weightsDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
        }
        _weights.CreateCommitedResource(weightsDesc);
        _weights.SetName("SSAO blur weights");

        float* weightsData = (float*)_weights.Map();
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
        _weights.Unmap();
    }

    void SSAOBlurPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Depth = builder.ReadResource(DEPTH);
        _data.AOTarget = builder.ReadResource("AO Target");

        dx12::ResourceDescription aoBlurDesc;
        {
            aoBlurDesc.SetSize(_camera->GetViewport().GetSize());
            aoBlurDesc.SetFormat(DXGI_FORMAT_R32_FLOAT);
            aoBlurDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.TempBlurTarget = builder.CreateResource("AO Blur", aoBlurDesc);
    }

    void SSAOBlurPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("SSAO Blur pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO Blur");
        {
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
            std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);
            std::shared_ptr<dx12::Resource> blurTarget = context.GetResource(_data.TempBlurTarget);

            D3D12_GPU_DESCRIPTOR_HANDLE depthHandle = context.GetGPUHandle(depth->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE aoTargetSRV = context.GetGPUHandle(aoTarget->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE blurTargetSRV = context.GetGPUHandle(blurTarget->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE aoTargetUAV = context.GetGPUHandle(aoTarget->GetAsUAV());
            D3D12_GPU_DESCRIPTOR_HANDLE blurTargetUAV = context.GetGPUHandle(blurTarget->GetAsUAV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { depth.get(),      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { aoTarget.get(),   D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { blurTarget.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_SSAOBlurHorizonralPipeline);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });
            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");

            CacheGPU::DataHandle cbHandle = context.GetCache().RequestPlacement("SSAOBlurPassCB", sizeof(ConstantsDesc));
            ConstantsDesc* cbDesc = (ConstantsDesc*)cbHandle.DataCPU;
            cbDesc->Radius = kRadius;
            cbDesc->DepthThreshold = kDepthThreshold;
            cbDesc->Sharpness = kSharpness;

            commandList.SetCBV(0, sceneDataHandle.DataGPU);
            commandList.SetCBV(1, cbHandle.DataGPU);
            commandList.SetSRV(2, _weights.OffsetGPU());
            commandList.SetDescriptorTable(3, depthHandle);
            commandList.SetDescriptorTable(4, aoTargetSRV);
            commandList.SetDescriptorTable(5, blurTargetUAV);

            XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { aoTarget.get(),   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { blurTarget.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_SSAOBlurVerticalPipeline);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });
            commandList.SetCBV(0, sceneDataHandle.DataGPU);
            commandList.SetCBV(1, cbHandle.DataGPU);
            commandList.SetSRV(2, _weights.OffsetGPU());
            commandList.SetDescriptorTable(3, depthHandle);
            commandList.SetDescriptorTable(4, blurTargetSRV);
            commandList.SetDescriptorTable(5, aoTargetUAV);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { depth.get(),      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { aoTarget.get(),   D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
                { blurTarget.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render