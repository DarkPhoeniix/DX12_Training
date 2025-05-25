#include "RendererPCH.h"

#include "SSAOComputePass.h"

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
        constexpr std::uint32_t kKernelSize = 16;
        constexpr float kRadius = 2.5f;
        constexpr float kBias = 0.025f;

        struct ConstantsDesc
        {
            std::uint32_t KernelSize;
            float Radius;
            float Bias;
        };
    } // namespace unnamed

    SSAOComputePass::SSAOComputePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<SSAOComputePassData>("SSAO Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
            _SSAOPipeline.Parse("PipelineDescriptions\\SSAOComputePipeline.tech");

            {
                dx12::ResourceDescription noiseDesc;
                {
                    noiseDesc.SetSize({ 64 * sizeof(XMVECTOR), 1});
                    noiseDesc.SetStride(sizeof(XMVECTOR));
                    noiseDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
                }
                _noise.CreateCommitedResource(noiseDesc);
                _noise.SetName("SSAO noise texture");
                dx12::ResourceDescription kernelsDesc;
                {
                    kernelsDesc.SetSize({ 16 * sizeof(XMVECTOR), 1 });
                    kernelsDesc.SetStride(sizeof(XMVECTOR));
                    kernelsDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
                }
                _kernels.CreateCommitedResource(kernelsDesc);
                _kernels.SetName("SSAO kernels");

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> dis(0.0, 1.0);

                XMVECTOR* noiseData = (XMVECTOR*)_noise.Map();
                for (size_t i = 0; i < 64; ++i)
                {
                    noiseData[i] = XMVectorSet(
                        dis(gen) * 2.0f - 1.0f,
                        dis(gen) * 2.0f - 1.0f,
                        0.0f,
                        0.0f
                    );

                    noiseData[i] = XMVector3Normalize(noiseData[i]);
                }
                _noise.Unmap();

                XMVECTOR* kernelsData = (XMVECTOR*)_kernels.Map();
                for (size_t i = 0; i < 16; ++i)
                {
                    kernelsData[i] = XMVectorSet(
                        dis(gen) * 2.0f - 1.0f,
                        dis(gen) * 2.0f - 1.0f,
                        dis(gen),
                        0.0f
                    );

                    kernelsData[i] = XMVector3Normalize(kernelsData[i]);

                    float scale = float(i) / float(16);
                    scale = std::lerp(0.1f, 1.0f, scale * scale);
                    kernelsData[i] *= scale;
                }
                _kernels.Unmap();
            }
    }

    void SSAOComputePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.NormalRoughness = builder.ReadResource(NORMAL_ROUGHNESS);
        _data.Depth = builder.ReadResource(DEPTH);

        dx12::ResourceDescription aoDesc;
        {
            aoDesc.SetSize(_camera->GetViewport().GetSize());
            aoDesc.SetFormat(DXGI_FORMAT_R32_FLOAT);
            aoDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.AOTarget = builder.CreateResource("AO Target", aoDesc);
    }

    void SSAOComputePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("SSAO pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "SSAO");
        {
            std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
            std::shared_ptr<dx12::Resource> aoTarget = context.GetResource(_data.AOTarget);

            D3D12_GPU_DESCRIPTOR_HANDLE normalSpecularHandle = context.GetGPUHandle(normalRoughness->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE depthHandle = context.GetGPUHandle(depth->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE aoTargetHandle = context.GetGPUHandle(aoTarget->GetAsUAV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { normalRoughness.get(),    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth.get(),              D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { &_noise,                  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { &_kernels,                D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { aoTarget.get(),           D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_SSAOPipeline);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
            CacheGPU::DataHandle cbHandle = context.GetCache().RequestPlacement("SSAOComputePassCB", sizeof(ConstantsDesc));
            ConstantsDesc* cbDesc = (ConstantsDesc*)cbHandle.DataCPU;
            cbDesc->KernelSize = kKernelSize;
            cbDesc->Radius = kRadius;
            cbDesc->Bias = kBias;

            commandList.SetCBV(0, sceneDataHandle.DataGPU);
            commandList.SetCBV(1, cbHandle.DataGPU);
            commandList.SetSRV(2, _kernels.OffsetGPU());
            commandList.SetSRV(3, _noise.OffsetGPU());
            commandList.SetDescriptorTable(4, depthHandle);
            commandList.SetDescriptorTable(5, normalSpecularHandle);
            commandList.SetDescriptorTable(6, aoTargetHandle);

            XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { normalRoughness.get(),    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { depth.get(),              D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { &_noise,                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { &_kernels,                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { aoTarget.get(),           D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
