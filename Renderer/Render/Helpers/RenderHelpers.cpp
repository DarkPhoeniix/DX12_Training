#include "RendererPCH.h"

#include "RenderHelpers.h"

#include "CommandList.h"
#include "ResourceTable.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Scene.h"
#include "Scene/SceneCache.h"

#include "Render/Frame/Frame.h"
#include "Render/Helpers/GPUStructs.h"

using namespace DirectX;

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

    XMMATRIX GetLightViewProj(std::shared_ptr<scene::Entity> node)
    {
        scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");
        scene::Light* light = node->GetComponentAs<scene::Light>("Light");

        XMMATRIX view;
        XMMATRIX proj;

        XMVECTOR lightDir = XMVector3Normalize(light->Direction);
        XMVECTOR lightPos = transform->Transform.r[3];
        XMVECTOR lightTar = lightPos + lightDir * light->Range;

        view = XMMatrixLookAtLH(lightPos, lightTar, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(light->OuterAngle), 1.0f, 0.5f, light->Range);

        return view * proj;
    }

    void SetupLightToGPU(std::shared_ptr<scene::Entity> node, CacheGPU::DataHandle& dataHandle, Frame* frame, uint32_t& index)
    {
        scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");
        scene::Light* light = node->GetComponentAs<scene::Light>("Light");

        if (light)
        {
            GPULightDesc* data = (GPULightDesc*)dataHandle.DataCPU;

            GPULightDesc lightDesc;
            {
                lightDesc.Position = transform->Transform.r[3];
                lightDesc.Direction = light->Direction;
                lightDesc.Color = light->Color;

                lightDesc.Range = light->Range;
                lightDesc.Intensity = light->Intensity;

                lightDesc.OuterAngle = std::cosf(XMConvertToRadians(light->OuterAngle * 0.5f));
                lightDesc.InnerAngle = std::cosf(XMConvertToRadians(light->InnerAngle * 0.5f));

                lightDesc.ViewProj= GetLightViewProj(node);

                lightDesc.Type = (uint32_t)light->Type;

                lightDesc.CastShadows = light->CastShadows;
                lightDesc.ShadowMapIndex = frame->GetResourceTable().GetResourceIndex(light->ShadowMap.get(), dx12::ResourceViewType::SRV);
            }

            data[index++] = lightDesc;
        }

        for (std::shared_ptr<scene::Entity>& child : node->GetChildrenNodes())
        {
            SetupLightToGPU(child, dataHandle, frame, index);
        }
    }
} // namespace unnamed

namespace Helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, dx12::CommandList& commandList, Frame* frame)
    {
        uint32_t lightsNum = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            CheckLightsNum(node, lightsNum);
        }

        // Setup scene data
        CacheGPU::DataHandle sceneDataHandle = frame->GetCache().RequestPlacement(sizeof(GPUSceneDesc));

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

            sceneDesc->InvView = XMMatrixInverse(nullptr, sceneDesc->View);
            sceneDesc->InvProjection = XMMatrixInverse(nullptr, sceneDesc->Projection);

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

        CacheGPU::DataHandle lightsData = frame->GetCache().RequestPlacement(sizeof(GPULightDesc) * lightsNum);

        uint32_t lightCounter = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            SetupLightToGPU(node, lightsData, frame, lightCounter);
        }

        commandList.SetSRV(2, lightsData.DataGPU);

    }
} // namespace Helpers