#include "RendererPCH.h"

#include "LightingPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Camera.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace render
{
    LightingPass::LightingPass(scene::Scene* scene, scene::Camera* camera)
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

        dx12::ResourceDescription targetDesc;
        {
            targetDesc.SetSize(_camera->GetViewport().GetSize());
            targetDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            targetDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        _data.HDRTarget = builder.CreateResource(HDR_TARGET, targetDesc);
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

            commandList.TransitionBarrier(*hdrTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(*albedoMetallic, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(*normalRoughness, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

            commandList.SetPipelineState(_deferredPipeline);

            helpers::SetupSceneDataGPU(*_scene, commandList, &context.GetCache());
            helpers::SetupLightDataGPU(*_scene, commandList, &context.GetCache(), context.GetResourceTable());

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

            commandList.TransitionBarrier(*hdrTarget, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*albedoMetallic, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*normalRoughness, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
