#include "RendererPCH.h"

#include "ShadowPass.h"

#include "ResourceTable.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"


namespace render
{
    void ShadowPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _shadowsPipeline.Parse("PipelineDescriptions\\PCFShadows.tech");
    }

    void ShadowPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void ShadowPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_shadowsPipeline);
        task->SetName("shadows");
        _tasks.push_back(task);

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Shadow pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Shadow Pass");
        {
            dx12::ResourceTable& sceneTable = *_scene->GetCache().GetTextureTable();
            dx12::ResourceTable& frameTable = _frame->GetResourceTable();

            auto lightEntities = _scene->FilterNodesByComponent("Light");
            for (uint32_t i = 0 ; i < lightEntities.size(); ++i)
            {
                scene::Light* light = lightEntities[i]->GetComponentAs<scene::Light>("Light");

                frameTable.CopyDescriptor(light->ShadowMap.get(), dx12::ResourceViewType::DSV, sceneTable);
                frameTable.CopyDescriptor(light->ShadowMap.get(), dx12::ResourceViewType::SRV, sceneTable);
                D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameTable.GetResourceCPUHandle(light->ShadowMap.get(), dx12::ResourceViewType::DSV);

                commandList.TransitionBarrier(*light->ShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);

                commandList.SetPipelineState(_shadowsPipeline);

                commandList.ClearDSV(depthHandle, D3D12_CLEAR_FLAG_DEPTH);
                commandList.SetViewport(scene::Viewport(light->ShadowMap->GetResourceDescription().GetSize()));
                commandList.SetRenderTargets({ }, &depthHandle);

                Helpers::SetupSceneDataGPU(*_scene, commandList, _frame);

                commandList.SetConstant(4, i);

                auto DrawEntity = [&commandList](std::shared_ptr<scene::Entity>& entity, CacheGPU& frameCache)
                    {
                        scene::SceneCache* cache = entity->GetSceneCache();
                        if (ASSERT(cache, "Entity has no scene cache"))
                        {
                            return;
                        }

                        scene::Animation* animation = entity->GetComponentAs<scene::Animation>("Animation");
                        scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
                        scene::Transformation transform = entity->GetGlobalTransform();
                        scene::Material* material = entity->GetComponentAs<scene::Material>("Material");
                        scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

                        if (!mesh)
                        {
                            return;
                        }

                        CacheGPU::DataHandle modelDescHandle = frameCache.RequestPlacement(sizeof(GPUModelDesc));

                        GPUModelDesc* modelDesc = (GPUModelDesc*)modelDescHandle.DataCPU;
                        {
                            modelDesc->Transform = transform.Transform;

                            if (material)
                            {
                                std::shared_ptr<dx12::ResourceTable> textureTable = entity->GetSceneCache()->GetTextureTable();

                                modelDesc->AlbedoTextureIndex = textureTable->GetResourceIndex(material->Albedo.get(), dx12::ResourceViewType::SRV);
                                modelDesc->NormalMapTextureIndex = textureTable->GetResourceIndex(material->NormalMap.get(), dx12::ResourceViewType::SRV);
                                modelDesc->MetalnessTextureIndex = textureTable->GetResourceIndex(material->Metalness.get(), dx12::ResourceViewType::SRV);
                                modelDesc->RoughnessTextureIndex = textureTable->GetResourceIndex(material->Roughness.get(), dx12::ResourceViewType::SRV);

                                if (armature)
                                {
                                    modelDesc->UseSkinning = true;
                                }
                            }
                        }

                        commandList.SetCBV(1, modelDescHandle.DataGPU);

                        // Update and setup animantion
                        if (armature && animation)
                        {
                            CacheGPU::DataHandle bonesDescHandle = frameCache.RequestPlacement(armature->BoneTransforms.GetResourceDescription().GetSize().x);

                            DirectX::XMMATRIX* data = (DirectX::XMMATRIX*)bonesDescHandle.DataCPU;

                            const auto& transforms = animation->GetBonesTransforms(cache->GetTime());
                            armature->ApplyAnimation(transforms);
                            armature->UpdateGlobalTransformations();

                            const std::vector<scene::Bone*>& bones = armature->GetSortedBones();
                            for (int i = 0; i < bones.size(); ++i)
                            {
                                DirectX::XMMATRIX result = bones[i]->Offset * bones[i]->GlobalTransform;
                                data[i] = result;
                            }

                            commandList.SetSRV(3, bonesDescHandle.DataGPU);
                        }

                        if (mesh)
                        {
                            commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                            commandList.SetVertexBuffer(0, mesh->VertexBufferView);
                            if (!mesh->SkinningVertexData.empty())
                            {
                                commandList.SetVertexBuffer(1, mesh->SkinningVertexBufferView);
                            }
                            commandList.SetIndexBuffer(mesh->IndexBufferView);

                            commandList.DrawIndexed(mesh->IndexData.size());
                        }
                    };

                for (std::shared_ptr<scene::Entity>& node : _scene->GetRootNodes())
                {
                    node->UpdateGlobalTransform();
                    DrawEntity(node, _frame->GetCache());

                    for (std::shared_ptr<scene::Entity>& child : node->GetChildrenNodes())
                    {
                        child->UpdateGlobalTransform(&node->GetGlobalTransform());
                        DrawEntity(node, _frame->GetCache());
                    }
                }

                commandList.TransitionBarrier(*light->ShadowMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
