#include "RendererPCH.h"

#include "LightingPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/Passes/PassResources.h"
#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    struct PassConstants
    {
        std::uint32_t AlbedoMetallicTextureIndex;
        std::uint32_t NormalRoughnessTextureIndex;
        std::uint32_t DepthTextureIndex;
        std::uint32_t TargetTextureIndex;
	};
}

namespace render
{
    LightingPass::LightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<LightingPassData>("Lighting Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _deferredPipeline.Parse("PipelineDescriptions\\DeferredShading.tech");
    }

    void LightingPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.FrameBuffer = builder.ReadResourceNew("Frame Buffer");

        _data.AlbedoMetallic = builder.ReadResourceNew(ALBEDO_METALLIC);
        _data.NormalRoughness = builder.ReadResourceNew(NORMAL_ROUGHNESS);
        _data.Depth = builder.ReadResourceNew(DEPTH);

        //std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        //size_t lightsNum = lightEntities.size();
        //
        //_data.ShadowMaps.resize(lightsNum, rg::ResourceId(-1));
        //for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
        //{
        //    std::shared_ptr<scene::Entity> entity = lightEntities[lightIndex];
        //    std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");
        //
        //    if (light->CastShadows)
        //    {
        //        _data.ShadowMaps[lightIndex] = builder.ReadResource(std::format("{}_ShadowMap", entity->GetName()));
        //    }
        //}

        _data.HDRTarget = builder.WriteResourceNew(HDR_TARGET);
    }

    void LightingPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Lighting pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
        {
            std::shared_ptr<dx12::Resource> frameBuffer     = context.GetResourceNew(_data.FrameBuffer);
            std::shared_ptr<dx12::Resource> hdrTarget       = context.GetResourceNew(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> albedoMetallic  = context.GetResourceNew(_data.AlbedoMetallic);
            std::shared_ptr<dx12::Resource> normalRoughness = context.GetResourceNew(_data.NormalRoughness);
            std::shared_ptr<dx12::Resource> depth           = context.GetResourceNew(_data.Depth);

            DescriptorHandle hdrTargetHandle                = context.GetStaticResourceHandle(hdrTarget->GetAsUAV());
            DescriptorHandle albedoMetallicHandle           = context.GetStaticResourceHandle(albedoMetallic->GetAsSRV());
            DescriptorHandle normalSpecularHandle           = context.GetStaticResourceHandle(normalRoughness->GetAsSRV());
            DescriptorHandle depthHandle                    = context.GetStaticResourceHandle(depth->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,              D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { albedoMetallic,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { normalRoughness,        D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth,                  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_deferredPipeline);

            PassConstants passCB =
            {
                .AlbedoMetallicTextureIndex = albedoMetallicHandle.Index,
                .NormalRoughnessTextureIndex = normalSpecularHandle.Index,
                .DepthTextureIndex = depthHandle.Index,
                .TargetTextureIndex = hdrTargetHandle.Index
            };

            commandList.SetCBV(0, frameBuffer->OffsetGPU());
			commandList.SetConstants(1, sizeof(PassConstants), &passCB);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { hdrTarget,              D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
                { albedoMetallic,         D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { normalRoughness,        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { depth,                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render