#include "RendererPCH.h"

#include "LightingPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/Passes/PassResources.h"
#include "Scene/Entity/Components/Light.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

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
        _data.AlbedoMetallic = builder.ReadResource(ALBEDO_METALLIC);
        _data.NormalRoughness = builder.ReadResource(NORMAL_ROUGHNESS);
        _data.Depth = builder.ReadResource(DEPTH);

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        size_t lightsNum = lightEntities.size();

        _data.ShadowMaps.resize(lightsNum, rg::ResourceId(-1));
        for (size_t lightIndex = 0; lightIndex < lightsNum; ++lightIndex)
        {
            std::shared_ptr<scene::Entity> entity = lightEntities[lightIndex];
            std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");

            if (light->CastShadows)
            {
                _data.ShadowMaps[lightIndex] = builder.ReadResource(std::format("{}_ShadowMap", entity->GetName()));
            }
        }

        _data.HDRTarget = builder.WriteResource(HDR_TARGET);
    }

    void LightingPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Lighting pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 4, "Deferred Shading");
        {
            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
            std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            D3D12_GPU_DESCRIPTOR_HANDLE hdrTargetHandle = context.GetGPUHandle(hdrTarget->GetAsUAV());
            D3D12_GPU_DESCRIPTOR_HANDLE albedoMetallicHandle = context.GetGPUHandle(albedoMetallic->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE normalSpecularHandle = context.GetGPUHandle(normalRoughness->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE depthHandle = context.GetGPUHandle(depth->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,              D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { albedoMetallic,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { normalRoughness,        D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth,                  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_deferredPipeline);

            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
            commandList.SetCBV(0, sceneDataHandle.DataGPU);

            CacheGPU::DataHandle lightsData = context.GetCache().GetResourcePlacement("LightsCB");
            commandList.SetSRV(2, lightsData.DataGPU);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            commandList.SetDescriptorTable(3, depthHandle);
            commandList.SetDescriptorTable(4, albedoMetallicHandle);
            commandList.SetDescriptorTable(5, normalSpecularHandle);
            commandList.SetDescriptorTable(6, context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetHeapStartGPUHandle());
            commandList.SetDescriptorTable(7, context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetHeapStartGPUHandle());
            commandList.SetDescriptorTable(8, hdrTargetHandle);

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