#include "RendererPCH.h"

#include "ShadowPass.h"

#include "ResourceTable.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"

#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"

namespace
{
    void DrawEntity(std::shared_ptr<scene::Entity> entity, dx12::CommandList& commandList, CacheGPU* frameCache)
    {
        if (scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            CacheGPU::DataHandle modelDescHandle = frameCache->GetResourcePlacement(entity->GetName());
            GPUModelDesc* desc = (GPUModelDesc*)modelDescHandle.DataCPU;
            commandList.SetCBV(1, modelDescHandle.DataGPU);

            // Update and setup animantion
            if (scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature"))
            {
                CacheGPU::DataHandle bonesDescHandle = frameCache->GetResourcePlacement(entity->GetName() + "_bones");
                commandList.SetSRV(3, bonesDescHandle.DataGPU);
            }

            commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            commandList.SetVertexBuffer(0, mesh->VertexBufferView);
            if (!mesh->SkinningVertexData.empty())
            {
                commandList.SetVertexBuffer(1, mesh->SkinningVertexBufferView);
            }
            commandList.SetIndexBuffer(mesh->IndexBufferView);

            commandList.DrawIndexed(mesh->IndexData.size());
        }

        for (std::shared_ptr<scene::Entity>& child : entity->GetChildrenNodes())
        {
            DrawEntity(child, commandList, frameCache);
        }
    }
} // namespace unnamed

namespace render
{
    void ShadowPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _shadowSpotLightPipeline.Parse("PipelineDescriptions\\Shadow_SpotLight.tech");
        _shadowPointLightPipeline.Parse("PipelineDescriptions\\Shadow_PointLight.tech");
    }

    void ShadowPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ShadowPass::Execute()
    {
        SpotLightsPass();
        PointLightsPass();
    }

    void ShadowPass::SpotLightsPass()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowSpotLightPipeline);
        task->SetName("shadows_spot");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Shadow pass (spot lights) command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 12, "Shadow Pass - Spot lights");
        {
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            auto lightEntities = _scene->FilterNodesByComponent("Light");
            for (uint32_t i = 0; i < lightEntities.size(); ++i)
            {
                scene::Light* light = lightEntities[i]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Spot)
                {
                    continue;
                }

                commandList.SetPipelineState(_shadowSpotLightPipeline);

                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    break;
                }

                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);
                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

                commandList.SetConstant(4, i);

                for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
                {
                    DrawEntity(node, commandList, &_frame->GetCache());
                }

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }

    void ShadowPass::PointLightsPass()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowPointLightPipeline);
        task->SetName("shadows_point");
        task->AddDependency("shadows_spot");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Shadow pass (point lights) command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 12, "Shadow Pass - Point lights");
        {
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            auto lightEntities = _scene->FilterNodesByComponent("Light");
            for (uint32_t i = 0; i < lightEntities.size(); ++i)
            {
                scene::Light* light = lightEntities[i]->GetComponentAs<scene::Light>("Light");
                if (light->Type != scene::LightType::Point)
                {
                    continue;
                }

                commandList.SetPipelineState(_shadowPointLightPipeline);

                std::shared_ptr<dx12::Resource> shadowMap = light->ShadowMap;
                if (!shadowMap)
                {
                    break;
                }

                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(shadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);
                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(shadowMap.get(), dx12::ResourceViewType::DSV);

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
                commandList.SetViewport(scene::Viewport(shadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

                commandList.SetConstant(4, i);

                for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
                {
                    DrawEntity(node, commandList, &_frame->GetCache());
                }

                commandList.TransitionBarrier(*shadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
