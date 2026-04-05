#include "RendererPCH.h"

#include "BloomApplyPass.h"

#include "Core/RenderSettings.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    namespace
    {
        struct PassConstants
        {
            std::uint32_t BloomTextureIndex;
            std::uint32_t HDRTextureIndex;
            float BloomIntensity;
        };
    }

    BloomApplyPass::BloomApplyPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BloomApplyPassData>("bloom_apply_pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _bloomApplyPipeline = _device->CreatePipelineState("PipelineDescriptions\\BloomApplyPipeline.tech");
    }

    void BloomApplyPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.HDRTarget = builder.WriteTexture("hdr_target");
        _data.Bloom = builder.ReadTexture("bloom_mip_1");
    }

    void BloomApplyPass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList, "Bloom Apply Pass", 9);

            commandList->SetComputePipelineState(_bloomApplyPipeline.get());

            PassConstants passCB =
            { 
                .BloomTextureIndex = context.GetBindlessIndex(_data.Bloom, rhi::ResourceViewType::SRV),
                .HDRTextureIndex = context.GetBindlessIndex(_data.HDRTarget, rhi::ResourceViewType::UAV),
                .BloomIntensity = RenderSettings::Bloom().Intensity 
            };
            commandList->SetComputeConstants(1, 3, &passCB);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 16.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 16.0f);
            commandList->Dispatch(xThreadGroups, yThreadGroups, 1);
        }

        commandList->Close();
    }
} // namespace render
