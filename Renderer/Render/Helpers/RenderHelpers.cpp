#include "RendererPCH.h"

#include "RenderHelpers.h"

#include "CommandList.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Scene.h"
#include "Scene/SceneCache.h"

#include "Render/Frame/CacheGPU.h"
#include "Render/Helpers/GPUStructs.h"

namespace
{
    void CheckLightsNum(std::shared_ptr<scene::Entity> node, uint32_t& lightsNum)
    {
        if (node->GetComponentAs<scene::Light>("Light"))
        {
            ++lightsNum;
        }

        for (std::shared_ptr<scene::Entity>& child : node->GetChildrenNodes())
        {
            CheckLightsNum(child, lightsNum);
        }
    }

    void SetupLightToGPU(std::shared_ptr<scene::Entity> node, CacheGPU::DataHandle& dataHandle, uint32_t& index)
    {
        scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");
        scene::Light* light = node->GetComponentAs<scene::Light>("Light");
        if (light)
        {
            GPULightDesc* data = (GPULightDesc*)dataHandle.DataCPU;

            GPULightDesc lightDesc;
            {
                lightDesc.position = transform->Transform.r[3];
                lightDesc.direction = light->Direction;
                lightDesc.color = light->Color;
                lightDesc.range = light->Range;
                lightDesc.intensity = light->Intensity;
                lightDesc.type = (uint32_t)light->Type;
            }

            data[index++] = lightDesc;
        }

        for (std::shared_ptr<scene::Entity>& child : node->GetChildrenNodes())
        {
            SetupLightToGPU(child, dataHandle, index);
        }
    }
} // namespace unnamed

namespace Helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache)
    {
        uint32_t lightsNum = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            CheckLightsNum(node, lightsNum);
        }

        // Setup scene data
        CacheGPU::DataHandle sceneDataHandle = frameCache->RequestPlacement(sizeof(GPUSceneDesc));

        GPUSceneDesc* sceneDesc = (GPUSceneDesc*)sceneDataHandle.DataCPU;
        {
            auto cameraEntity = scene.FindNodeByComponentName("Camera");
            if (ASSERT(cameraEntity.get(), "No camera on the scene"))
            {
                return;
            }

            scene::Camera* camera = cameraEntity->GetComponentAs<scene::Camera>("Camera");

            sceneDesc->View = camera->View();
            sceneDesc->Projection = camera->Projection();
            sceneDesc->ViewProjection = camera->ViewProjection();

            sceneDesc->InvView = DirectX::XMMatrixInverse(nullptr, sceneDesc->View);
            sceneDesc->InvProjection = DirectX::XMMatrixInverse(nullptr, sceneDesc->Projection);

            sceneDesc->EyeDirection = camera->Look();
            sceneDesc->EyePosition = camera->Position();

            const scene::Viewport& viewport = camera->GetViewport();
            sceneDesc->WindowSize = {
                (uint32_t)viewport.GetSize().x,
                (uint32_t)viewport.GetSize().y
            };
            sceneDesc->ReciprocalWindowSize = {
                (1.0f / (float)viewport.GetSize().x),
                (1.0f / (float)viewport.GetSize().y)
            };
            sceneDesc->NearFar = { camera->GetNearZ(), camera->GetFarZ() };

            sceneDesc->LightsNum = lightsNum;
        }

        commandList.SetCBV(0, sceneDataHandle.DataGPU);

        CacheGPU::DataHandle lightsData = frameCache->RequestPlacement(sizeof(GPULightDesc) * lightsNum);

        uint32_t lightCounter = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            SetupLightToGPU(node, lightsData, lightCounter);
        }

        commandList.SetSRV(2, lightsData.DataGPU);

    }
} // namespace Helpers