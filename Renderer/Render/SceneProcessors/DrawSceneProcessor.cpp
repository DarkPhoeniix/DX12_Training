#include "stdafx.h"

#include "DrawSceneProcessor.h"

#include "Scene/Scene.h"
#include "DXObjects/CommandList.h"

void DrawSceneProcessor::Process(SceneLayer::Scene& scene, Core::CommandList& commandList)
{
    // Setup textures
    commandList.SetDescriptorHeaps({ scene.GetCache().GetTextureTable()->GetDescriptorHeap().GetDXDescriptorHeap().Get()});
    commandList.SetDescriptorTable(3, scene.GetCache().GetTextureTable()->GetDescriptorHeap().GetHeapStartGPUHandle());

    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        DrawEntity(*node, commandList);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            DrawEntity(*child, commandList);
        }
    }
}

void DrawSceneProcessor::DrawEntity(SceneLayer::Entity& entity, Core::CommandList& commandList)
{
    SceneLayer::SceneCache* cache = entity.GetSceneCache();
    if (ASSERT(cache, "Entity has no scene cache"))
    {
        return;
    }

    Transformation* transform = entity.GetComponentAs<Transformation>("Transformation");
    Material* material = entity.GetComponentAs<Material>("Material");
    Mesh* mesh = entity.GetComponentAs<Mesh>("Mesh");

    GPUModelDesc* modelDesc = (GPUModelDesc*)entity.GetGPUDesc().Map();
    {
        modelDesc->Transform = transform->Transform;

        if (material)
        {
            std::shared_ptr<Core::ResourceTable> textureTable = entity.GetSceneCache()->GetTextureTable();

            modelDesc->AlbedoTextureIndex = textureTable->GetResourceIndex(material->Albedo->GetName());
            modelDesc->NormalMapTextureIndex = textureTable->GetResourceIndex(material->NormalMap->GetName());
            modelDesc->MetalnessTextureIndex = textureTable->GetResourceIndex(material->Metalness->GetName());
            modelDesc->RoughnessTextureIndex = textureTable->GetResourceIndex(material->Roughness->GetName());
        }
    }

    commandList.SetCBV(1, entity.GetGPUDesc().OffsetGPU(0));

    if (mesh)
    {
        commandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList.SetVertexBuffer(0, mesh->VertexBufferView);
        commandList.SetIndexBuffer(mesh->IndexBufferView);

        commandList.DrawIndexed(mesh->IndexData.size());
    }
}
