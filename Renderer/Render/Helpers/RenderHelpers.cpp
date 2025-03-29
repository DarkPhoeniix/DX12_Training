#include "RendererPCH.h"

#include "RenderHelpers.h"

#include "CommandList.h"
#include "ResourceTable.h"

#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Scene.h"

#include "Render/Frame/CacheGPU.h"
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

    std::array<XMMATRIX, 6> GetLightViewProj(std::shared_ptr<scene::Entity> node)
    {
        scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");
        scene::Light* light = node->GetComponentAs<scene::Light>("Light");

        std::array<XMMATRIX, 6> result = 
        {
            XMMatrixIdentity(),
            XMMatrixIdentity(),
            XMMatrixIdentity(),
            XMMatrixIdentity(),
            XMMatrixIdentity(),
            XMMatrixIdentity()
        };

        if (light->Type == scene::LightType::Spot)
        {
            XMMATRIX view;

            XMVECTOR lightDir = XMVector3Normalize(light->Direction);
            XMVECTOR lightPos = transform->Transform.r[3];
            XMVECTOR lightTar = lightPos + lightDir * light->Range;

            view = XMMatrixLookAtLH(lightPos, lightTar, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

            result[0] = view;
        }
        else if (light->Type == scene::LightType::Point)
        {
            XMVECTOR lightPos = transform->Transform.r[3];
            XMVECTOR lightTar = lightPos + XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMMATRIX view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[0] = view;

            lightTar = lightPos + XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
            up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[1] = view;

            lightTar = lightPos + XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            up = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[2] = view;

            lightTar = lightPos + XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
            up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[3] = view;

            lightTar = lightPos + XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[4] = view;

            lightTar = lightPos + XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            view = XMMatrixLookAtLH(lightPos, lightTar, up);

            result[5] = view;
        }

        return result;
    }

    void SetupLightToGPU(std::shared_ptr<scene::Entity> node, CacheGPU::DataHandle& dataHandle, dx12::ResourceTable& frameTable, uint32_t& index)
    {
        scene::Transformation* transform = node->GetComponentAs<scene::Transformation>("Transformation");
        scene::Light* light = node->GetComponentAs<scene::Light>("Light");

        if (light)
        {
            GPULightDesc* data = (GPULightDesc*)dataHandle.DataCPU;

            GPULightDesc lightDesc;
            {
                lightDesc.Position = transform->Transform.r[3];
                lightDesc.Direction = XMVector3Normalize(light->Direction);
                lightDesc.Color = light->Color;

                lightDesc.Range = light->Range;
                lightDesc.Intensity = light->Intensity;

                lightDesc.OuterAngle = std::cosf(XMConvertToRadians(light->OuterAngle * 0.5f));
                lightDesc.InnerAngle = std::cosf(XMConvertToRadians(light->InnerAngle * 0.5f));

                lightDesc.ViewProj = GetLightViewProj(node);
                switch (light->Type)
                {
                case scene::LightType::Spot:
                {
                    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(light->OuterAngle), 1.0f, 1.0f, light->Range);
                    lightDesc.ViewProj[0] *= proj;
                    lightDesc.PerspectiveValues[0] = proj.r[2].m128_f32[2];
                    lightDesc.PerspectiveValues[1] = proj.r[3].m128_f32[2];
                }
                break;
                case scene::LightType::Point:
                {
                    XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), 1.0f, 0.5f, light->Range);
                    for (size_t i = 0; i < lightDesc.ViewProj.size(); ++i)
                    {
                        lightDesc.ViewProj[i] *= proj;
                    }
                    lightDesc.PerspectiveValues[0] = proj.r[2].m128_f32[2];
                    lightDesc.PerspectiveValues[1] = proj.r[3].m128_f32[2];
                }
                break;
                }

                lightDesc.Type = (uint32_t)light->Type;

                lightDesc.CastShadows = light->CastShadows;
                if (light->CastShadows)
                {
                    lightDesc.ShadowMapIndex = frameTable.GetResourceIndex(std::format("{}_ShadowMap", node->GetName()), dx12::ResourceViewType::SRV);
                }
            }

            data[index++] = lightDesc;
        }

        for (std::shared_ptr<scene::Entity>& child : node->GetChildrenNodes())
        {
            SetupLightToGPU(child, dataHandle, frameTable, index);
        }
    }
} // namespace unnamed

namespace helpers
{
    void SetupSceneDataGPU(scene::Scene& scene, CacheGPU* cache)
    {
        uint32_t lightsNum = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            CheckLightsNum(node, lightsNum);
        }

        // Setup scene data
        CacheGPU::DataHandle sceneDataHandle = cache->RequestPlacement("SceneCB", sizeof(GPUSceneDesc));

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
            sceneDesc->NearFar = { camera->NearZ, camera->FarZ };

            sceneDesc->LightsNum = lightsNum;
        }
    }

    void SetupLightDataGPU(scene::Scene& scene, CacheGPU* cache, dx12::ResourceTable& resourceTable)
    {
        uint32_t lightsNum = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            CheckLightsNum(node, lightsNum);
        }
        CacheGPU::DataHandle lightsData = cache->RequestPlacement("LightsCB", sizeof(GPULightDesc) * lightsNum);

        uint32_t lightCounter = 0;
        for (std::shared_ptr<scene::Entity>& node : scene.GetRootNodes())
        {
            SetupLightToGPU(node, lightsData, resourceTable, lightCounter);
        }
    }
} // namespace helpers