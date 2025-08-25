#include "RendererPCH.h"

#include "SkyboxPass.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Skybox.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

namespace
{
    struct PassConstants
    {
        std::uint32_t DepthTextureIndex;
        std::uint32_t SkyboxTextureIndex;
        std::uint32_t TargetTextureIndex;
    };
}

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
        _data.Depth = builder.ReadResource("depth_target");
        _data.HDRTarget = builder.WriteResource("hdr_target");
    }

    void SkyboxPass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Skybox pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Skybox Pass");
        {
            std::shared_ptr<scene::Entity> entity = _scene->FindNodeByComponentName("Skybox");
            std::shared_ptr<scene::Skybox> skyboxComponent = entity->GetComponentAs<scene::Skybox>("Skybox");

            std::shared_ptr<dx12::Resource> skybox = context.GetTextureManager().GetTexture(skyboxComponent->SkydomeTextureHandle);
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.HDRTarget);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            DescriptorHandle targetHandle = context.GetStaticResourceHandle(target->GetAsUAV());
            DescriptorHandle depthHandle = context.GetStaticResourceHandle(depth->GetAsSRV());
            DescriptorHandle skyboxHandle = context.GetStaticResourceHandle(skybox->GetAsSRV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { target, D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { skybox, D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { depth,  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE }
            };
            commandList.TransitionBarriers(barriers);

            context.BindBindlessTable(commandList);
            commandList.SetPipelineState(_skyboxPipeline);

            commandList.SetCBV(0, context.GetFrame()->_frameBuffer->OffsetGPU());
            PassConstants passCB =
            {
                .DepthTextureIndex = depthHandle.Index,
				.SkyboxTextureIndex = skyboxHandle.Index,
				.TargetTextureIndex = targetHandle.Index
            };
			commandList.SetConstants(1, 3, &passCB);

            DirectX::XMUINT2 viewportSize = _camera->GetViewport().GetSize();
            int xThreadGroups = (uint32_t)std::ceilf(viewportSize.x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(viewportSize.y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);

            barriers =
            {
                { target, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON },
                { skybox, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { depth,  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
