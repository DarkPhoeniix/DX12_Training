#include "RendererPCH.h"

#include "SetupCachedDataProcessor.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Transformation.h"
#include "Scene/Scene.h"

#include "Render/Frame/CacheGPU.h"
#include "Render/GPUStructs/GPULightDesc.h"
#include "Render/GPUStructs/GPUSceneDesc.h"

void SetupCachedDataProcessor::Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    SceneLayer::SceneCache& cache = scene.GetCache();

    _lightNum = 0;

    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        ProcessEntity(*node, commandList, frameCache);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            ProcessEntity(*child, commandList, frameCache);
        }
    }

    commandList.SetSRV(2, cache.GetLightsSRV().OffsetGPU(0));

    // Setup scene data
    GPUSceneDesc* sceneDesc = (GPUSceneDesc*)scene.GetGPUDesc().Map();
    {
        auto cameraEntity = scene.FindNodeByComponentName("Camera");
        if (ASSERT(cameraEntity.get(), "No camera on the scene"))
        {
            return;
        }

        SceneLayer::Camera* camera = cameraEntity->GetComponentAs<SceneLayer::Camera>("Camera");

        sceneDesc->View = camera->View();
        sceneDesc->Projection = camera->Projection();
        sceneDesc->ViewProjection = camera->ViewProjection();

        sceneDesc->InvView = DirectX::XMMatrixInverse(nullptr, sceneDesc->View);
        sceneDesc->InvProjection = DirectX::XMMatrixInverse(nullptr, sceneDesc->Projection);

        sceneDesc->EyeDirection = camera->Look();
        sceneDesc->EyePosition = camera->Poisition();

        const SceneLayer::Viewport& viewport = camera->GetViewport();
        sceneDesc->WindowSize = { 
            (uint32_t)viewport.GetSize().x, 
            (uint32_t)viewport.GetSize().y 
        };
        sceneDesc->ReciprocalWindowSize = { 
            (1.0f / (float)viewport.GetSize().x), 
            (1.0f / (float)viewport.GetSize().y) 
        };
        sceneDesc->NearFar = { camera->GetNearZ(), camera->GetFarZ() };

        sceneDesc->LightsNum = _lightNum;
    }

    commandList.SetCBV(0, scene.GetGPUDesc().OffsetGPU(0));
}

void SetupCachedDataProcessor::ProcessEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache)
{
    SceneLayer::SceneCache* cache = entity.GetSceneCache();
    if (ASSERT(cache, "Entity has no scene cache"))
    {
        return;
    }

    SceneLayer::Transformation* transform = entity.GetComponentAs<SceneLayer::Transformation>("Transformation");
    SceneLayer::Light* light = entity.GetComponentAs<SceneLayer::Light>("Light");
    if (light)
    {
        GPULightDesc* data = (GPULightDesc*)cache->GetLightsSRV().Map();

        GPULightDesc lightDesc;
        {
            lightDesc.position = transform->Transform.r[3];
            lightDesc.direction = light->Direction;
            lightDesc.color = light->Color;
            lightDesc.range = light->Range;
            lightDesc.intensity = light->Intensity;
            lightDesc.type = (uint32_t)light->Type;
        }

        data[_lightNum++] = lightDesc;
    }
}
