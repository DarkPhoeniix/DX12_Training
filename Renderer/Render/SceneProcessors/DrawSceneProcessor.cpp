#include "stdafx.h"

#include "DrawSceneProcessor.h"

#include "CommandList.h"
#include "ResourceTable.h"

#include "Scene/Scene.h"
#include "Scene/ECS/Components/Animation.h"
#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Material.h"
#include "Scene/ECS/Components/Mesh.h"
#include "Scene/ECS/Components/Transformation.h"

#include "Render/GPUStructs/GPUModelDesc.h"
#include "Render/Frame/CacheGPU.h"

void DrawSceneProcessor::Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    // Setup textures
    commandList.SetDescriptorHeaps({ scene.GetCache().GetTextureTable()->GetDescriptorHeap().GetDXDescriptorHeap().Get() });
    commandList.SetDescriptorTable(4, scene.GetCache().GetTextureTable()->GetDescriptorHeap().GetHeapStartGPUHandle());

    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        node->UpdateGlobalTransform();
        DrawEntity(*node, commandList, frameCache);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            child->UpdateGlobalTransform(&node->GetGlobalTransform());
            DrawEntity(*child, commandList, frameCache);
        }
    }
}

void DrawSceneProcessor::DrawEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache, SceneLayer::Entity* parent)
{
    SceneLayer::SceneCache* cache = entity.GetSceneCache();
    if (ASSERT(cache, "Entity has no scene cache"))
    {
        return;
    }

    Transformation transform = entity.GetGlobalTransform();
    Material* material = entity.GetComponentAs<Material>("Material");
    Mesh* mesh = entity.GetComponentAs<Mesh>("Mesh");

    GPUModelDesc* modelDesc = (GPUModelDesc*)entity.GetGPUDesc().Map();
    {
        modelDesc->Transform = transform.Transform;

        if (material)
        {
            std::shared_ptr<dx12::ResourceTable> textureTable = entity.GetSceneCache()->GetTextureTable();

            modelDesc->AlbedoTextureIndex = textureTable->GetResourceIndex(material->Albedo->GetName());
            modelDesc->NormalMapTextureIndex = textureTable->GetResourceIndex(material->NormalMap->GetName());
            modelDesc->MetalnessTextureIndex = textureTable->GetResourceIndex(material->Metalness->GetName());
            modelDesc->RoughnessTextureIndex = textureTable->GetResourceIndex(material->Roughness->GetName());
        }
    }

    commandList.SetCBV(1, entity.GetGPUDesc().OffsetGPU(0));

    Animation* animation = entity.GetComponentAs<Animation>("Animation");
    Armature* armature = entity.GetComponentAs<Armature>("Armature");
    // Update and setup animantion
    if (armature && animation)
    {
        CacheGPU::DataHandle dataHandle = frameCache->RequestPlacement(armature->BoneTransforms.GetResourceDescription().GetSize().x);

        DirectX::XMMATRIX* data = (DirectX::XMMATRIX*)dataHandle.DataCPU;

        const auto& transforms = animation->GetBonesTransforms(cache->GetTime());
        armature->ApplyAnimation(transforms);
        armature->UpdateGlobalTransformations();

        const std::vector<Bone*>& bones = armature->GetSortedBones();
        for (int i = 0; i < bones.size(); ++i)
        {
            DirectX::XMMATRIX result = bones[i]->Offset * bones[i]->GlobalTransform;
            data[i] = result;
        }

        commandList.SetSRV(3, dataHandle.DataGPU);
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
}
