#include "RendererPCH.h"

#include "UpdateSceneProcessor.h"

#include "Scene/Scene.h"
#include "Scene/ECS/Components/Animation.h"
#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Material.h"
#include "Scene/ECS/Components/Mesh.h"
#include "Scene/ECS/Components/Transformation.h"

#include "Render/Frame/CacheGPU.h"

void UpdateSceneProcessor::Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        UpdateEntity(*node, commandList, frameCache);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            UpdateEntity(*child, commandList, frameCache);
        }
    }
}

void UpdateSceneProcessor::UpdateEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    Armature* armature = entity.GetComponentAs<Armature>("Armature");

    if (armature)
    {
        for (Bone& bone : armature->GetBones())
        {

        }
    }
}
