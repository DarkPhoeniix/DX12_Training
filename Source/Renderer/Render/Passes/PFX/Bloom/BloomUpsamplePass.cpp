#include "RendererPCH.h"

#include "BloomUpsamplePass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    static constexpr std::uint32_t MAX_MIP_LEVELS = 6;
}

namespace render
{
    BloomUpsamplePass::BloomUpsamplePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomUpsamplePassData>("bloom_upsample_pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomUpsamplePipeline.Parse("PipelineDescriptions\\BloomUpsamplePipeline.tech");
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

    void BloomUpsamplePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("bloom_upsample_pass_cmd_list");

        {
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 8, "Bloom Upsample Pass");

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_bloomUpsamplePipeline);

            for (std::uint32_t mip = _mipCount - 1; mip > 0; --mip)
            {
                std::shared_ptr<dx12::Resource> bloomATarget = context.GetResource(_data.BloomMips[mip]);
                std::shared_ptr<dx12::Resource> bloomBTarget = context.GetResource(_data.BloomMips[mip - 1]);
                DescriptorHandle bloomATargetHandle = context.GetStaticResourceHandle(bloomATarget->GetAsSRV());
                DescriptorHandle bloomBTargetHandle = context.GetStaticResourceHandle(bloomBTarget->GetAsUAV());

                struct PassConstants
                {
                    std::uint32_t InputTextureIndex;
                    std::uint32_t OutputTextureIndex;
                    float FilterRadius;
                    float Intensity;
                } passCB{ .InputTextureIndex = bloomATargetHandle.Index, .OutputTextureIndex = bloomBTargetHandle.Index, .FilterRadius = RenderSettings::Bloom().Radius, .Intensity = RenderSettings::Bloom().Intensity1 };
                commandList.SetConstants(1, 4, &passCB);

                std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().x / 16.0f);
                std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(bloomBTarget->GetResourceDescription().GetSize().y / 16.0f);
                commandList.Dispatch(xThreadGroups, yThreadGroups, 1);

                commandList.UAVBarrier(bloomBTarget);
            }
        }

        commandList.Close();
    }
} // namespace render
