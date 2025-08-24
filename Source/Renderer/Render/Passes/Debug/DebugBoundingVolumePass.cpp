#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"

#include "Render/Helpers/DrawHelpers.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPassBuilder.h"

#include "ResourceBarrier.h"

namespace render
{
    DebugBoundingVolumePass::DebugBoundingVolumePass(std::shared_ptr<scene::Scene> scene, scene::Camera* camera)
        : RenderPass<DebugBoundingVolumePassData>("Debug Volumes Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
    }

    void DebugBoundingVolumePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.WriteResourceNew("render_target");
        _data.Depth = builder.ReadResourceNew("depth_target");
    }

    void DebugBoundingVolumePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render Debug Volumes command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Debug Volumes");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResourceNew(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResourceNew(_data.Depth);

            DescriptorHandle rtv = context.GetStaticResourceHandle(target->GetAsRTV());
            DescriptorHandle dsv = context.GetStaticResourceHandle(depth->GetAsDSV());

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { target, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET },
                { depth,  D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetViewport(_camera->GetViewport());
            commandList.SetRenderTarget(&rtv.CpuHandle, &dsv.CpuHandle);

            auto lights = _scene->FilterNodesByComponent("Light");
            for (auto& entity : lights)
            {
                std::shared_ptr<scene::Light> light = entity->GetComponentAs<scene::Light>("Light");
                std::shared_ptr<scene::Transformation> t = entity->GetComponentAs<scene::Transformation>("Transformation");

                switch (light->Type)
                {
                case scene::LightType::Point:
                    DrawHelper::DrawSphere(commandList, *_camera, light->Range, t->Transform.r[3], light->Color);
                    break;
                case scene::LightType::Spot:
                    DrawHelper::DrawCone(commandList, *_camera, light->OuterAngle, light->Range, t->Transform.r[3], light->Direction, light->Color);
                    break;
                }
            }

            auto meshes = _scene->FilterNodesByComponent("Mesh");
            for (auto& entity : meshes)
            {
                std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");
                scene::AABBVolume aabb = mesh->GlobalAABB;

                DrawHelper::DrawBox(commandList, *_camera, aabb.Min, aabb.Max, DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
            }

            barriers =
            {
                { target, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON},
                { depth,  D3D12_RESOURCE_STATE_DEPTH_WRITE,   D3D12_RESOURCE_STATE_COMMON}
            };
            commandList.TransitionBarriers(barriers);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
