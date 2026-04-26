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
        : RenderPass<BloomDownsamplePassData>(device, "bloom_downsample_pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomDownsamplePass1Pipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomDownsamplePass1Pipeline.tech");
        _bloomDownsamplePipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomDownsamplePipeline.tech");
    }

    void BloomDownsamplePass::Setup(rg::RenderPassBuilder& builder)
    {
        DirectX::XMUINT2 viewportSize = _camera->GetSize();

        std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2;
        std::uint32_t maxMipCount = std::floor(std::log2(size));
        _mipCount = std::min(maxMipCount - 1, MAX_MIP_LEVELS);
        rhi::TextureDescription brightnessDesc =
        {
            .Format = rhi::Format::R16G16B16A16_FLOAT,
            .Dimension = rhi::TextureDimension::Texture2D,
            .Flags = rhi::ResourceFlags::AllowUnorderedAccess
        };
        for (std::uint32_t i = 1; i < (_mipCount + 1); ++i)
        {
            brightnessDesc.Width = viewportSize.x / std::pow(2, i);
            brightnessDesc.Height = viewportSize.y / std::pow(2, i);
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

            std::shared_ptr<rhi::Texture> bloomTarget = context.GetTexture(_data.BloomMips[0]);

            commandList->SetComputePipelineState(_bloomDownsamplePass1Pipeline.get());

                PassConstants passCB =
                { 
                    .InputTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::SRV),
                    .OutputTextureIndex = context.GetBindlessIndex(_data.BloomMips[0], rhi::ResourceViewType::UAV),
                    .Gamma = RenderSettings::ToneMapping().Gamma 
                };
                commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
                commandList->SetComputeConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetWidth() / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomTarget->GetHeight() / 16.0f);
                commandList->Dispatch(xThreadGroups, yThreadGroups, 1);

            commandList->SetComputePipelineState(_bloomDownsamplePipeline.get());

            for (std::uint32_t mip = 0; mip < _mipCount - 1; ++mip)
            {
                std::shared_ptr<rhi::Texture> bloomBTarget = context.GetTexture(_data.BloomMips[mip + 1]);

                PassConstants passCB =
                { 
                    .InputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip], rhi::ResourceViewType::SRV),
                    .OutputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip + 1], rhi::ResourceViewType::UAV),
                    .Gamma = RenderSettings::ToneMapping().Gamma 
                };
                commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
                commandList->SetComputeConstants(1, 3, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetWidth() / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetHeight() / 16.0f);
                commandList->Dispatch(xThreadGroups, yThreadGroups, 1);
            }
        }

        commandList->Close();
    }
} // namespace render
