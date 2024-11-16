#include "stdafx.h"

#include "SetupCachedDataProcessor.h"

#include "Scene/Scene.h"
#include "Render/GPUStructs/GPUSceneDesc.h"
#include "DXObjects/CommandList.h"

void SetupCachedDataProcessor::Process(SceneLayer::Scene& scene, Core::CommandList& commandList)
{
    SceneLayer::SceneCache& cache = scene.GetCache();

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

        sceneDesc->LightsNum = cache.GetLightManager()->GetLightsNum();
    }

    commandList.SetCBV(0, scene.GetGPUDesc().OffsetGPU(0));

    // Setup lights
    cache.GetLightManager()->SetupLights(commandList);
}
