#include "RendererPCH.h"

#include "LightingPass.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    struct PassConstants
    {
        std::uint32_t AlbedoMetallicTextureIndex;
        std::uint32_t NormalRoughnessTextureIndex;
        std::uint32_t EmissionTextureIndex;
        std::uint32_t DepthTextureIndex;
        std::uint32_t TargetTextureIndex;
    };
}

namespace render
{
    LightingPass::LightingPass(rhi::Device* device, std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<LightingPassData>(device, "lighting_pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _deferredPipeline = _device->CreatePipelineState("PipelineDescriptions\\DeferredShading.tech");
    }

    void LightingPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.AlbedoMetallic = builder.ReadTexture("albedo_metallic_target");
        _data.NormalRoughness = builder.ReadTexture("normal_roughness_target");
        _data.Emission = builder.ReadTexture("emission_target");
        _data.Depth = builder.DepthStencilRead("depth_target");

        _data.HDRTarget = builder.WriteTexture("hdr_target");
    }

    void LightingPass::Execute(rg::RenderContext& context, rg::ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();

        {
            GPU_SCOPED_EVENT(commandList, "Deferred Shading Pass", 4);

            commandList->SetComputePipelineState(_deferredPipeline.get());

            PassConstants passCB =
            {
                .AlbedoMetallicTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV),
                .NormalRoughnessTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV),
                .EmissionTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV),
                .DepthTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::SRV),
                .TargetTextureIndex = context.GetBindlessIndex(_data.AlbedoMetallic, rhi::ResourceViewType::UAV)
            };

            commandList->SetComputeConstants(1, 5, &passCB);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList->Dispatch(xThreadGroups, yThreadGroups);
        }

        commandList->Close();
    }
} // namespace render