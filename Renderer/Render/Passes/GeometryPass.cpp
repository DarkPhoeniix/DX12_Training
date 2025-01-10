#include "RendererPCH.h"

#include "GeometryPass.h"

#include "ResourceTable.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"
#include "Utility/DebugInfo.h"

namespace render
{
    void GeometryPass::Inititalize()
    {
        IRenderPass::Inititalize();

        _name = "GeometryPass";

        _geometryPipeline.Parse("PipelineDescriptions\\GPassPipeline.tech");
    }

    void GeometryPass::Destroy()
    {
        IRenderPass::Destroy();
    }

    void GeometryPass::Execute()
    {
        TaskGPU* task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, &_geometryPipeline);
        task->SetName("g-pass");
        task->AddDependency("clear");

        dx12::CommandList& commandList = *task->GetCommandLists().front();
        commandList.SetName("Geometry pass command list");

        PIXBeginEvent(commandList.GetDXCommandList().Get(), 2, "Geometry Pass");
        {
            D3D12_CPU_DESCRIPTOR_HANDLE albedoMetalnessHandle = _gBuffer->GetAlbedoMetalnessTextureCPUHandle();
            D3D12_CPU_DESCRIPTOR_HANDLE normalSpecularHandle = _gBuffer->GetNormalTextureCPUHandle();
            D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = _gBuffer->GetDepthTextureCPUHandle();

            commandList.SetPipelineState(_geometryPipeline);

            commandList.SetViewport(_activeCamera->GetViewport());
            commandList.SetRenderTargets({ albedoMetalnessHandle, normalSpecularHandle }, &depthHandle);

#if defined(_DEBUG)
            DebugInfo::StartStatCollecting(commandList);
#endif

            Helpers::SetupSceneDataGPU(*_scene, commandList, &_frame->GetCache());

            // Setup textures
            commandList.SetDescriptorHeaps({ _scene->GetCache().GetTextureTable()->GetDescriptorHeap().GetDXDescriptorHeap().Get() });
            commandList.SetDescriptorTable(4, _scene->GetCache().GetTextureTable()->GetDescriptorHeap().GetHeapStartGPUHandle());

            auto DrawEntity = [&commandList](std::shared_ptr<scene::Entity>& entity, CacheGPU& frameCache)
                {
                    scene::SceneCache* cache = entity->GetSceneCache();
                    if (ASSERT(cache, "Entity has no scene cache"))
                    {
                        return;
                    }

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
                        }
                    }

                    commandList.SetCBV(1, modelDescHandle.DataGPU);

                    scene::Animation* animation = entity->GetComponentAs<scene::Animation>("Animation");
                    scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
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

#if defined(_DEBUG)
            DebugInfo::EndStatCollecting(commandList);
#endif

            commandList.TransitionBarrier(_gBuffer->GetDepthTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer->GetAlbedoMetalnessTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            commandList.TransitionBarrier(_gBuffer->GetNormalTexture(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        }
        PIXEndEvent(commandList.GetDXCommandList().Get());

        commandList.Close();
    }
} // namespace render
