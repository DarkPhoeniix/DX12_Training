#include "RendererPCH.h"

#include "BloomApplyPass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    BloomApplyPass::BloomApplyPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomApplyPassData>("bloom_apply_pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomApplyPipeline.Parse("PipelineDescriptions\\BloomApplyPipeline.tech");
    }

    void BloomApplyPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.WriteTexture("hdr_target");
        _data.Bloom = builder.ReadTexture("bloom_mip_1");
    }

    void BloomApplyPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("bloom_apply_pass_cmd_list");

        {
            PIXScopedEvent(commandList.GetDXCommandList().Get(), 9, "Bloom Apply Pass");

            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> bloomTarget = context.GetResource(_data.Bloom);

            DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsUAV());
            DescriptorHandle bloomTargetHandle = context.GetStaticResourceHandle(bloomTarget->GetAsSRV());

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_bloomApplyPipeline);

            struct PassConstants
            {
                std::uint32_t BloomTextureIndex;
                std::uint32_t HDRTextureIndex;
                float BloomIntensity;
            } passCB{ .BloomTextureIndex = bloomTargetHandle.Index, .HDRTextureIndex = hdrTargetHandle.Index, .BloomIntensity = RenderSettings::Bloom().Intensity };
            commandList.SetConstants(1, 3, &passCB);

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(hdrTarget->GetResourceDescription().GetSize().x / 16.0f);
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(hdrTarget->GetResourceDescription().GetSize().y / 16.0f);
            commandList.Dispatch(xThreadGroups, yThreadGroups, 1);
        }

        commandList.Close();
    }
} // namespace render
