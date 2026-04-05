#include "RendererPCH.h"

#include "BloomDownsamplePass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    namespace
    {
        static constexpr std::uint32_t MAX_MIP_LEVELS = 6;

        struct PassConstants
        {
            std::uint32_t InputTextureIndex;
            std::uint32_t OutputTextureIndex;

            float Gamma;
        };
    } // namespace unnamed

    BloomDownsamplePass::BloomDownsamplePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomDownsamplePassData>("bloom_downsample_pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomDownsamplePass1Pipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomDownsamplePass1Pipeline.tech");
        _bloomDownsamplePipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomDownsamplePipeline.tech");
    }

    void BloomDownsamplePass::Setup(rg::RenderPassBuilder& builder)
    {
        DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();

        std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2;
        std::uint32_t maxMipCount = std::floor(std::log2(size));
        _mipCount = std::min(maxMipCount - 1, MAX_MIP_LEVELS);
        rhi::ResourceDescription brightnessDesc;
        {
            brightnessDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            brightnessDesc.SetResourceType(rhi::ResourceType::Texture | rhi::ResourceType::Unordered);
        }
        for (std::uint32_t i = 1; i < (_mipCount + 1); ++i)
        {
            std::uint32_t targetWidth = viewportSize.x / std::pow(2, i);
            std::uint32_t targetHeight = viewportSize.y / std::pow(2, i);
            brightnessDesc.SetSize({ targetWidth, targetHeight });
            builder.DeclareTexture("bloom_mip_" + std::to_string(i), brightnessDesc);

            _data.BloomMips.push_back(builder.WriteTexture("bloom_mip_" + std::to_string(i)));
        }

        _data.HDRTarget = builder.ReadTexture("hdr_target");
    }

    void BloomDownsamplePass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList, "Bloom Downsample Pass", 8);

            commandList->SetComputePipelineState(_bloomDownsamplePass1Pipeline.get());

            {
                PassConstants passCB =
                { 
                    .InputTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::SRV),
                    .OutputTextureIndex = context.GetBindlessIndex(_data.BloomMips[0], rhi::ResourceViewType::UAV),
                    .Gamma = RenderSettings::ToneMapping().Gamma 
                };
                commandList->SetComputeConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetResourceDescription().GetSize().x / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetResourceDescription().GetSize().y / 16.0f);
                commandList->Dispatch(xThreadGroups, yThreadGroups, 1);
            }

            commandList->SetComputePipelineState(_bloomDownsamplePipeline.get());

            for (std::uint32_t mip = 0; mip < _mipCount - 1; ++mip)
            {
                PassConstants passCB =
                { 
                    .InputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip], rhi::ResourceViewType::SRV),
                    .OutputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip + 1], rhi::ResourceViewType::UAV),
                    .Gamma = RenderSettings::ToneMapping().Gamma 
                };
                commandList->SetComputeConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().x / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().y / 16.0f);
                commandList->Dispatch(xThreadGroups, yThreadGroups, 1);
            }
        }

        commandList->Close();
    }
} // namespace render
