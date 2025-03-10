#include "RendererPCH.h"

#include "SkyboxPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/Helpers/RenderHelpers.h"
#include "Render/Passes/PassResources.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Skybox.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace render
{
    SkyboxPass::SkyboxPass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<SkyboxPassData>("Skybox Pass", rg::RenderPassType::Compute)
        , _scene(scene)
        , _camera(camera)
    {
        _skyboxPipeline.Parse("PipelineDescriptions\\SkyboxPipeline.tech");
    }

    void SkyboxPass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Depth = builder.ReadResource(DEPTH);
        _data.HDRTarget = builder.WriteResource(HDR_TARGET);
    }

    void SkyboxPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Skybox pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Skybox Pass");
        {
            std::shared_ptr<scene::Entity> entity = _scene->FindNodeByComponentName("Skybox");
            scene::Skybox* skyboxC = entity->GetComponentAs<scene::Skybox>("Skybox");

            std::shared_ptr<dx12::Resource> skybox = skyboxC->SkydomeTexture;
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            D3D12_GPU_DESCRIPTOR_HANDLE targetHandle = context.GetGPUHandle(target->GetAsUAV());
            D3D12_GPU_DESCRIPTOR_HANDLE depthHandle = context.GetGPUHandle(depth->GetAsSRV());
            D3D12_GPU_DESCRIPTOR_HANDLE skyboxHandle = context.GetGPUHandle(skybox->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { target.get(), D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { skybox.get(), D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth.get(),  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_skyboxPipeline);

            helpers::SetupSceneDataGPU(*_scene, commandList, &context.GetCache());

            // Setup textures
            commandList.SetDescriptorHeaps({ context.GetResourceTable().GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get() });

            commandList.SetDescriptorTable(3, depthHandle);
            commandList.SetDescriptorTable(4, skyboxHandle);
            commandList.SetDescriptorTable(5, targetHandle);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { target.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { skybox.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { depth.get(),  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
