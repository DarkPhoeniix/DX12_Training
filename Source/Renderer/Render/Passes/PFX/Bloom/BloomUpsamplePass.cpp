#include "RendererPCH.h"

#include "BloomUpsamplePass.h"

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
            float FilterRadius;
            float Intensity;
        };
    } // namespace unnamed

    BloomUpsamplePass::BloomUpsamplePass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomUpsamplePassData>(device, "bloom_upsample_pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomUpsamplePipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomUpsamplePipeline.tech");
    }

    void BloomUpsamplePass::Setup(rg::RenderPassBuilder& builder)
    {
        DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
        std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2;
        std::uint32_t maxMipCount = std::floor(std::log2(size));
        _mipCount = std::min(maxMipCount - 1, MAX_MIP_LEVELS);
        for (std::uint32_t i = 1; i < (_mipCount + 1); ++i)
        {
            _data.BloomMips.push_back(builder.WriteTexture("bloom_mip_" + std::to_string(i)));
        }
    }

    void BloomUpsamplePass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList, "Bloom Upsample Pass", 8);

            commandList->SetComputePipelineState(_bloomUpsamplePipeline.get());

            for (std::uint32_t mip = _mipCount - 1; mip > 0; --mip)
            {
                std::shared_ptr<rhi::Texture> bloomBTarget = context.GetTexture(_data.BloomMips[mip - 1]);

                PassConstants passCB =
                { 
                    .InputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip], rhi::ResourceViewType::SRV),
                    .OutputTextureIndex = context.GetBindlessIndex(_data.BloomMips[mip - 1], rhi::ResourceViewType::UAV),
                    .FilterRadius = RenderSettings::Bloom().Radius, 
                    .Intensity = RenderSettings::Bloom().Intensity1 
                };
                commandList->SetComputeCBV(0, context.GetFrameBuffer()->GetVirtualAddress());
                commandList->SetComputeConstants(1, 4, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetWidth() / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetHeight() / 16.0f);
                commandList->Dispatch(xThreadGroups, yThreadGroups, 1);

                commandList->UAVBarrier(bloomBTarget);
            }
        }

        commandList->Close();
    }
} // namespace render
