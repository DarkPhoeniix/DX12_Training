#include "RendererPCH.h"

#include "UpdateSceneProcessor.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Transformation.h"

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
    SceneLayer::Armature* armature = entity.GetComponentAs<SceneLayer::Armature>("Armature");

    if (armature)
    {
        for (SceneLayer::Bone& bone : armature->GetBones())
        {

        }
    }
}
