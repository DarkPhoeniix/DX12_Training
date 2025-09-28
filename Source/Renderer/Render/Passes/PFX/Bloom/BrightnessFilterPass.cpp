#include "RendererPCH.h"

#include "BrightnessFilterPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    BrightnessFilterPass::BrightnessFilterPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<BrightnessFilterPassData>("Brightness Filter Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _brightnessFilterPipeline.Parse("PipelineDescriptions\\BrightnessFilterPipeline.tech");
    }

    void BrightnessFilterPass::Setup(rg::RenderPassBuilder& builder)
    {
        dx12::ResourceDescription brightnessDesc;
        {
            brightnessDesc.SetSize(_camera->GetViewport().GetSize());
            brightnessDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            brightnessDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        builder.DeclareTexture("brightness_target", brightnessDesc);

        _data.HDRTarget = builder.ReadTexture("hdr_target");
        _data.BrightnessTarget = builder.WriteTexture("brightness_target");
    }
    
    void BrightnessFilterPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Brightness filter pass command list");
        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Brightness Filter Pass");
        {
            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> brightnessTarget = context.GetResource(_data.BrightnessTarget);

            DescriptorHandle hdrTargetHandle = context.GetStaticResourceHandle(hdrTarget->GetAsSRV());
            DescriptorHandle brightnessTargetHandle = context.GetStaticResourceHandle(brightnessTarget->GetAsUAV());

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_brightnessFilterPipeline);

            struct PassConstants
            {
                std::uint32_t HDRTextureIndex;
                std::uint32_t BrightnessTextureIndex;
                float Threshold;
            } passCB{ .HDRTextureIndex = hdrTargetHandle.Index, .BrightnessTextureIndex = brightnessTargetHandle.Index, .Threshold = 2.0f };
            commandList.SetConstants(1, 3, &passCB);

            std::uint32_t xThreadGroups = (std::uint32_t)std::ceilf(brightnessTarget->GetResourceDescription().GetSize().x / 16.0f);
            std::uint32_t yThreadGroups = (std::uint32_t)std::ceilf(brightnessTarget->GetResourceDescription().GetSize().y / 16.0f);
            commandList.Dispatch(xThreadGroups, yThreadGroups, 1);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
