#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"
#include "RendererPCH.h"

#include "DebugBoundingVolumePass.h"

#include "ResourceTable.h"

#include "Editor.h"
#include "Scene/Entity/Components/Camera.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/DrawHelpers.h"
#include "Utility/DebugInfo.h"

#include "RenderGraph/RenderPassBuilder.h"
#include "RenderGraph/RenderContext.h"

#include "Render/Passes/PassResources.h"

namespace render
{
    DebugBoundingVolumePass::DebugBoundingVolumePass(scene::Scene* scene, scene::Camera* camera)
        : RenderPass<DebugBoundingVolumePassData>("Debug Volumes Pass", rg::RenderPassType::Graphics)
        , _scene(scene)
        , _camera(camera)
    {
    }

    void DebugBoundingVolumePass::Setup(rg::RenderPassBuilder& builder)
    {
        _data.Target = builder.WriteResource(TARGET);
        _data.Depth = builder.ReadResource(DEPTH);
    }

    void DebugBoundingVolumePass::Execute(rg::RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Render Debug Volumes command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 5, "Debug Volumes");
        {
            std::shared_ptr<dx12::Resource> target = context.GetResource(_data.Target);
            std::shared_ptr<dx12::Resource> depth = context.GetResource(_data.Depth);

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = context.GetCPUHandle(target->GetAsRTV());
            D3D12_CPU_DESCRIPTOR_HANDLE dsv = context.GetCPUHandle(depth->GetAsDSV());

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_DEPTH_WRITE);

            commandList.SetViewport(_camera->GetViewport());
            commandList.SetRenderTarget(&rtv, &dsv);

            auto lights = _scene->FilterNodesByComponent("Light");
            for (auto& entity : lights)
            {
                scene::Light* light = entity->GetComponentAs<scene::Light>("Light");
                scene::Transformation* t = entity->GetComponentAs<scene::Transformation>("Transformation");

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
                scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh");
                scene::Transformation* t = entity->GetComponentAs<scene::Transformation>("Transformation");
                scene::AABBVolume aabb = mesh->AABB.Transform(t->Transform);

                DrawHelper::DrawBox(commandList, *_camera, aabb.Min, aabb.Max, DirectX::XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));
            }

            commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
            commandList.TransitionBarrier(*depth, D3D12_RESOURCE_STATE_COMMON);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
