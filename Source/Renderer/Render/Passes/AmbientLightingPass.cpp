#include "RendererPCH.h"

#include "AmbientLightingPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/RenderSettings.h"
#include "Render/Passes/PassResources.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    AmbientLightingPass::AmbientLightingPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<AmbientLightingPassData>("Ambient lighting Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        if (RenderSettings::UseIBL())
        {
            _ambientLightingPipeline.Parse("PipelineDescriptions\\AmbientLightingIBLPipeline.tech");
        }
        else
        {
            _ambientLightingPipeline.Parse("PipelineDescriptions\\AmbientLightingPipeline.tech");
        }
    }

    void AmbientLightingPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.AlbedoMetallic = builder.ReadResource(ALBEDO_METALLIC);
        builder.ReadResourceNew(ALBEDO_METALLIC);
        _data.NormalRoughness = builder.ReadResource(NORMAL_ROUGHNESS);
        builder.ReadResourceNew(NORMAL_ROUGHNESS);
        _data.Depth = builder.ReadResource(DEPTH);
        builder.ReadResourceNew(DEPTH);

        _data.DiffuseIrradianceMap = builder.ReadResource("Diffuse irradiance map");
        builder.ReadResourceNew("Diffuse irradiance map");
        _data.PreFilteredMap = builder.ReadResource("Prefiltered environment map");
        builder.ReadResourceNew("Prefiltered environment map");
        _data.BRDF_LUT = builder.ReadResource("BRDF LUT");
        builder.ReadResourceNew("BRDF LUT");

        dx12::ResourceDescription targetDesc;
        {
            targetDesc.SetSize(_camera->GetViewport().GetSize());
            targetDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.HDRTarget = builder.CreateResource(HDR_TARGET, targetDesc);
        builder.CreateResourceNew(HDR_TARGET, targetDesc);
    }

    void AmbientLightingPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Ambient pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 3, "Ambient Lighting");
        {
            std::shared_ptr<dx12::Resource> hdrTarget = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> albedoMetallic = context.GetResource(_data.AlbedoMetallic);
            std::shared_ptr<dx12::Resource> normalRoughness = context.GetResource(_data.NormalRoughness);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);
            std::shared_ptr<dx12::Resource> diffuseIrradianceMap = context.GetResource(_data.DiffuseIrradianceMap);
            std::shared_ptr<dx12::Resource> preFilteredEnv = context.GetResource(_data.PreFilteredMap);
            std::shared_ptr<dx12::Resource> brdfLUT = context.GetResource(_data.BRDF_LUT);

            D3D12_GPU_DESCRIPTOR_HANDLE hdrTargetHandle = context.GetGPUHandle(hdrTarget->GetAsUAV());
            D3D12_GPU_DESCRIPTOR_HANDLE albedoMetallicHandle = context.GetGPUHandle(albedoMetallic->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE normalSpecularHandle = context.GetGPUHandle(normalRoughness->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE depthHandle = context.GetGPUHandle(depth->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE diffuseIrradianceMapHandle = context.GetGPUHandle(diffuseIrradianceMap->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE preFilteredEnvHandle = context.GetGPUHandle(preFilteredEnv->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE brdfLUTHandle = context.GetGPUHandle(brdfLUT->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { hdrTarget,              D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { albedoMetallic,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { normalRoughness,        D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth,                  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { diffuseIrradianceMap,   D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { preFilteredEnv,         D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { brdfLUT,                D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_ambientLightingPipeline);

            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            CacheGPU::DataHandle sceneDataHandle = context.GetCache().GetResourcePlacement("SceneCB");
            commandList.SetCBV(0, sceneDataHandle.DataGPU);

            commandList.SetDescriptorTable(3, depthHandle);
            commandList.SetDescriptorTable(4, albedoMetallicHandle);
            commandList.SetDescriptorTable(5, normalSpecularHandle);
            commandList.SetDescriptorTable(6, diffuseIrradianceMapHandle);
            commandList.SetDescriptorTable(7, preFilteredEnvHandle);
            commandList.SetDescriptorTable(8, brdfLUTHandle);
            commandList.SetDescriptorTable(9, hdrTargetHandle);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { hdrTarget,              D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
                { albedoMetallic,         D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { normalRoughness,        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { depth,                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { diffuseIrradianceMap,   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { preFilteredEnv,         D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { brdfLUT,                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render