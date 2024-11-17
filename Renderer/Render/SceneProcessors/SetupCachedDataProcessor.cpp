#include "stdafx.h"

#include "SetupCachedDataProcessor.h"

#include "DXObjects/CommandList.h"
#include "Scene/Scene.h"
#include "Render/GPUStructs/GPUSceneDesc.h"
#include "Render/GPUStructs/GPULightDesc.h"

void SetupCachedDataProcessor::Process(SceneLayer::Scene& scene, Core::CommandList& commandList)
{
    SceneLayer::SceneCache& cache = scene.GetCache();

    for (std::shared_ptr<SceneLayer::Entity>& node : scene.GetRootNodes())
    {
        ProcessEntity(*node, commandList);

        for (std::shared_ptr<SceneLayer::Entity>& child : node->GetChildrenNodes())
        {
            ProcessEntity(*child, commandList);
        }
    }

    commandList.SetSRV(2, cache.GetLightsSRV().OffsetGPU(0));

    // Setup scene data
    GPUSceneDesc* sceneDesc = (GPUSceneDesc*)scene.GetGPUDesc().Map();
    {
        SceneLayer::Camera* camera = cache.GetCamera();

        sceneDesc->View = camera->View();
        sceneDesc->Projection = camera->Projection();
        sceneDesc->ViewProjection = camera->ViewProjection();

        sceneDesc->InvView = DirectX::XMMatrixInverse(nullptr, sceneDesc->View);
        sceneDesc->InvProjection = DirectX::XMMatrixInverse(nullptr, sceneDesc->Projection);

        sceneDesc->EyeDirection = camera->Look();
        sceneDesc->EyePosition = camera->Poisition();

        const SceneLayer::Viewport& viewport = camera->GetViewport();
        sceneDesc->WindowSize = { (uint32_t)viewport.GetSize().x, (uint32_t)viewport.GetSize().y };
        sceneDesc->NearFar = { camera->GetNearZ(), camera->GetFarZ() };

        sceneDesc->LightsNum = _lightNum;
    }

    commandList.SetCBV(0, scene.GetGPUDesc().OffsetGPU(0));
}

void SetupCachedDataProcessor::ProcessEntity(SceneLayer::Entity& entity, Core::CommandList& commandList)
{
    SceneLayer::SceneCache* cache = entity.GetSceneCache();
    if (ASSERT(cache, "Entity has no scene cache"))
    {
        return;
    }

    Transformation* transform = entity.GetComponentAs<Transformation>("Transformation");
    Light* light = entity.GetComponentAs<Light>("Light");
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
