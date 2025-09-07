#include "RendererPCH.h"

#include "BloomDownsamplePass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    static constexpr std::uint32_t MAX_MIP_LEVELS = 6;
}

namespace render
{
    BloomDownsamplePass::BloomDownsamplePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomDownsamplePassData>("Bloom Downsample Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomDownsamplePass1Pipeline.Parse("PipelineDescriptions\\BloomDownsamplePass1Pipeline.tech");
        _bloomDownsamplePipeline.Parse("PipelineDescriptions\\BloomDownsamplePipeline.tech");
    }

    void BloomDownsamplePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.ReadResource("hdr_target");

        DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();

        std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2;
        std::uint32_t maxMipCount = std::floor(std::log2(size));
        _mipCount = std::min(maxMipCount - 1, MAX_MIP_LEVELS);
        dx12::ResourceDescription brightnessDesc;
        {
            brightnessDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            brightnessDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        for (std::uint32_t i = 1; i < (_mipCount + 1); ++i)
        {
            std::uint32_t targetWidth = viewportSize.x / std::pow(2, i);
            std::uint32_t targetHeight = viewportSize.y / std::pow(2, i);
            brightnessDesc.SetSize({ targetWidth, targetHeight });
            _data.BloomMips.push_back(builder.CreateResource("bloom_mip_" + std::to_string(i), brightnessDesc));
        }
    }

    void BloomDownsamplePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Bloom downsample pass command list");
        PIXBeginEvent(commandList.GetDXCommandList().Get(), 8, "Bloom Downsample Pass");
        {
            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);

            DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_bloomDownsamplePass1Pipeline);

            {
                std::shared_ptr<dx12::Resource> bloomTarget = context.GetResource(_data.BloomMips[0]);

                DescriptorHandle bloomTargetHandle = context.GetStaticResourceHandle(bloomTarget->GetAsUAV());

                barriers =
                {
                    { bloomTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
                };
                commandList.TransitionBarriers(barriers);

                struct PassConstants
                {
                    std::uint32_t InputTextureIndex;
                    std::uint32_t OutputTextureIndex;

                    float Gamma;
                } passCB{ .InputTextureIndex = hdrTargetHandle.Index, .OutputTextureIndex = bloomTargetHandle.Index, .Gamma = RenderSettings::ToneMapping().Gamma };
                commandList.SetConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetResourceDescription().GetSize().x / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetResourceDescription().GetSize().y / 16.0f);
                commandList.Dispatch(xThreadGroups, yThreadGroups, 1);

                barriers =
                {
                    { bloomTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
                };
                commandList.TransitionBarriers(barriers);
            }

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_bloomDownsamplePipeline);

            for (std::uint32_t mip = 0; mip < _mipCount - 1; ++mip)
            {
                std::shared_ptr<dx12::Resource> bloomATarget = context.GetResource(_data.BloomMips[mip]);
                std::shared_ptr<dx12::Resource> bloomBTarget = context.GetResource(_data.BloomMips[mip + 1]);

                DescriptorHandle bloomATargetHandle = context.GetStaticResourceHandle(bloomATarget->GetAsSRV());
                DescriptorHandle bloomBTargetHandle = context.GetStaticResourceHandle(bloomBTarget->GetAsUAV());

                barriers =
                {
                    { bloomBTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
                };
                commandList.TransitionBarriers(barriers);

                struct PassConstants
                {
                    std::uint32_t InputTextureIndex;
                    std::uint32_t OutputTextureIndex;

                    float Gamma;
                } passCB{ .InputTextureIndex = bloomATargetHandle.Index, .OutputTextureIndex = bloomBTargetHandle.Index, .Gamma = RenderSettings::ToneMapping().Gamma };
                commandList.SetConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().x / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().y / 16.0f);
                commandList.Dispatch(xThreadGroups, yThreadGroups, 1);

                barriers =
                {
                    { bloomBTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON }
                };
                commandList.TransitionBarriers(barriers);
            }

            barriers =
            {
                { hdrTarget,        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
