
#include "RendererPCH.h"

#include "Renderer.h"

#include "RHI/CommandList.h"

#include "GPUCrashTracker/IGPUCrashTracker.h"

#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"

#include "Scene/SceneLoader.h"
#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Entity.h"
#include "Helpers/DebugInfo.h"

#include "Render/Helpers/GPUStructs.h"

#include "Core/RenderSettings.h"
#include "Render/Frame/TaskGPU.h"
#include "Render/Passes/PFX/AmbientOcclusion/SSAOApplyPass.h"
#include "Render/Passes/PFX/AmbientOcclusion/SSAOBlurPass.h"
#include "Render/Passes/PFX/AmbientOcclusion/SSAOComputePass.h"
#include "Render/Passes/Debug/DebugArmaturePass.h"
#include "Render/Passes/Debug/DebugBoundingVolumePass.h"
#include "Render/Passes/Debug/Views/DebugAlbedoViewPass.h"
#include "Render/Passes/Debug/Views/DebugEmissiveViewPass.h"
#include "Render/Passes/Debug/Views/DebugMetallicViewPass.h"
#include "Render/Passes/Debug/Views/DebugNormalViewPass.h"
#include "Render/Passes/Debug/Views/DebugRoughnessViewPass.h"
#include "Render/Passes/Debug/Views/DebugSSAOViewPass.h"
#include "Render/Passes/AmbientLightingPass.h"
#include "Render/Passes/GeometryPass.h"
#include "Render/Passes/LightingPass.h"
#include "Render/Passes/PFX/AntiAliasing/FXAAPass.h"
#include "Render/Passes/PFX/Bloom/BloomApplyPass.h"
#include "Render/Passes/PFX/Bloom/BloomDownsamplePass.h"
#include "Render/Passes/PFX/Bloom/BloomUpsamplePass.h"
#include "Render/Passes/PFX/ToneMapping/AverageLuminancePass.h"
#include "Render/Passes/PFX/ToneMapping/LuminanceHistogramPass.h"
#include "Render/Passes/PFX/ToneMapping/ToneMappingPass.h"
#include "Render/Passes/Shadows/ShadowCullPass.h"
#include "Render/Passes/Shadows/ShadowDrawPass.h"
#include "Render/Passes/SkyboxPass.h"
#include "Render/Helpers/DrawHelpers.h"

#include "RenderGraph/RenderPassBuilder.h"

#include "RHI/CommandQueue.h"
#include "RHI/SwapChain.h"
#include "RHI/ResourceBarrier.h"

using namespace DirectX;
using namespace core;

namespace
{
    constexpr char DEFAULT_SCENE_PATH[] = "Assets\\Sponza\\Sponza.scene";

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

    std::array<XMMATRIX, 6> GetLightViews(std::shared_ptr<scene::Entity> node)
    {
        std::shared_ptr<scene::Transformation> transform = node->GetComponentAs<scene::Transformation>("Transformation");
        ASSERT(transform, "Node does not have Transformation component");

        std::shared_ptr<scene::Light> light = node->GetComponentAs<scene::Light>("Light");
        ASSERT(light, "Node does not have Light component");

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

            XMVECTOR lightDir = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
            lightDir = XMVector3TransformNormal(lightDir, transform->Transform);
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
}

namespace render
{
    DXRenderer::DXRenderer(rhi::Device* device, HWND windowHandle)
        : _windowHandle(windowHandle)
        , _currentFrame(nullptr)
        , _device(device)
        , _contentLoaded(false)
        , _isMinimized(false)
        , _isCameraMoving(false)
        , _enableAbsoluteMovement(false)
        , _deltaTime(0.0f)
        , _scene(std::make_shared<scene::Scene>())
        , _gpuProfiler(device)
        , _sceneLoader(device)
    {
        DescriptorHeapManager::Create(_device, 2048, 128, 4096, 1024);
        ResourceTable::Create(_device);
		TextureManager::Create(_device);
        GeometryCacheManager::Create();

        _renderGraph = std::make_unique<rg::RenderGraph>(_device, &ResourceTable::Get());

		_sceneLoader.Init(ResourceTable::Get(), TextureManager::Get());

#if ENABLE_CPU_PROFILING || ENABLE_GPU_PROFILING
        _renderGraph->SetProfiler(&_gpuProfiler);
#endif // ENABLE_CPU_PROFILING || ENABLE_GPU_PROFILING
    }

    DXRenderer::~DXRenderer()
    {
        WaitAllFrames();
    }

    rg::RenderGraph* DXRenderer::GetRenderGraph()
    {
        return _renderGraph.get();
    }

    std::shared_ptr<scene::Scene> DXRenderer::GetCurrentScene()
    {
        return _scene;
    }

    bool DXRenderer::LoadContent(TaskGPU* uploadTask, const std::string& filepath)
    {
        render::DrawHelper::Init(_device);

        uploadTask->SetName("Upload Data");
        rhi::CommandList* commandList = uploadTask->GetCommandList();

        {
            TextureManager::Get().Clear();
            GeometryCacheManager::Get().Clear();
            DescriptorHeapManager::Get().Reset();
        }

        {
            // Load scene
            if (std::filesystem::exists(std::filesystem::path(filepath)))
            {
                _sceneLoader.LoadScene(*uploadTask, filepath, _scene);
            }
            else
            {
                LOG_WARNING("Failed to load scene \'{}\'. Falling back to the \'{}\' scene", filepath, DEFAULT_SCENE_PATH);
                _sceneLoader.LoadScene(*uploadTask, DEFAULT_SCENE_PATH, _scene);
            }

            // Camera Setup
            RECT windowSize;
            GetClientRect(_windowHandle, &windowSize);
            uint32_t windowWidth = windowSize.right - windowSize.left;
            uint32_t windowHeight = windowSize.bottom - windowSize.top;

            std::shared_ptr<scene::Entity> camera = _scene->FindNodeByComponentName("Camera");
            if (!camera)
            {
                camera = std::make_shared<scene::Entity>();
                camera->SetName("Default camera");

                std::shared_ptr<scene::Transformation> transform = std::make_shared<scene::Transformation>();
                std::shared_ptr<scene::Camera> cameraComponent = std::make_shared<scene::Camera>(windowWidth, windowHeight);
                cameraComponent->LookAt(XMVectorSet(5.0f, 1.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
                cameraComponent->SetLens(60.0f, 0.1f, 1000.0f);
                cameraComponent->SetSpeed(1.0f);

                camera->AddComponent(transform);
                camera->AddComponent(cameraComponent);
                _scene->AddRootNode(camera);
            }
            _cameraComponent = camera->GetComponentAs<scene::Camera>("Camera");
            _cameraComponent->SetSize(windowWidth, windowHeight);

            rhi::BufferDescription frameBufferDesc =
            {
                .Size = sizeof(GPUFrameDesc),
                .Usage = rhi::ResourceUsage::Upload
            };
            for (size_t i = 0; i < rhi::BACK_BUFFER_COUNT; ++i)
            {
                std::shared_ptr<rhi::Buffer> frameBuffer = _device->CreateBuffer(frameBufferDesc, rhi::ResourceState::GenericRead, std::format("frame_buffer_{}", i));

                _currentFrame->SetBuffer(frameBuffer);

                _currentFrame = _currentFrame->Next;
            }

            // Generate textures for IBL
            _diffuseIrradianceMap = _sceneLoader.GenerateEnvironmentDiffuseIrradianceMap(commandList, _scene);
            _brdfLUT = _sceneLoader.GenerateEnvironmentBRDFLookUpTexture(commandList, _scene);
            _preFilteredEnvironmentMap = _sceneLoader.GeneratePreFilteredEnvironmentMap(commandList, _scene);
        }

        commandList->Close();

        CreateShadowMaps();
        SetupRenderPipeline();

        _contentLoaded = true;
        return _contentLoaded;
    }

    void DXRenderer::UnloadContent()
    {
        render::DrawHelper::Destroy();
        TextureManager::Destroy();
        GeometryCacheManager::Destroy();
        ResourceTable::Destroy();
        DescriptorHeapManager::Destroy();

        _contentLoaded = false;
    }

    void DXRenderer::SetFrame(Frame& frame)
    {
        _currentFrame = &frame;
    }

    void DXRenderer::OnUpdate(events::UpdateEvent& updateEvent)
    {
        DebugInfo::BeginUpdate(updateEvent);

        _deltaTime = updateEvent.elapsedTime;

        DescriptorHeapManager::Get().AdvanceFrameIndex();
        ResourceTable::Get().ResetTransientResources();

        for (const auto& entity : _scene->GetRootNodes())
        {
            UpdateEntity(entity);
        }

        DebugInfo::EndUpdate();
    }

    void DXRenderer::OnRender(events::RenderEvent& renderEvent)
    {
        _currentFrame->WaitCPU();
        _currentFrame->ResetGPU();

        // Clear marker map for current frame before execution
        tracking::IGPUCrashTracker* crashTracker = _device->GetCrashTracker();
        crashTracker->AdvanceFrame();
        crashTracker->ResetMarkerMapForCurrentFrame();

        DebugInfo::BeginRender(renderEvent);

        _renderGraph->SetTaskAllocator(_currentFrame);
        _renderGraph->SetFrameBuffer(_currentFrame->GetBuffer().get());

        UpdateSceneBuffers();

        if (_isMinimized)
        {
            return;
        }

        _renderGraph->Execute();

        DebugInfo::EndRender();
    }

    void DXRenderer::OnKeyDown(events::KeyEvent& e)
    {
        XMVECTOR cameraMovement = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        XMVECTOR front = _cameraComponent->Look();
        XMVECTOR right = _cameraComponent->Right();
        XMVECTOR up = _cameraComponent->Up();

        if (_enableAbsoluteMovement)
        {
            front = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
            right = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
            up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        }

        if (e.keyCode == DIKeyCode::DIK_W)
        {
            cameraMovement += front * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_S)
        {
            cameraMovement -= front * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_D)
        {
            cameraMovement += right * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_A)
        {
            cameraMovement -= right * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_SPACE)
        {
            cameraMovement += up * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_LSHIFT)
        {
            cameraMovement -= up * _deltaTime;
        }
        _cameraComponent->Update(cameraMovement);
    }

    void DXRenderer::OnKeyPressed(events::KeyEvent& e)
    {
        switch (e.keyCode)
        {
        case DIKeyCode::DIK_ESCAPE:
            ::SendMessage(_windowHandle, WM_DESTROY, 0, 0);
            break;
        case DIKeyCode::DIK_F1:
            _enableAbsoluteMovement = !_enableAbsoluteMovement;
            break;
        }
    }

    void DXRenderer::OnKeyReleased(events::KeyEvent& e)
    {
    }

    void DXRenderer::OnMouseMoved(events::MouseMoveEvent& e)
    {
        if ((e.relativeX != 0 || e.relativeY != 0) && _isCameraMoving)
        {
            _cameraComponent->Update(e.relativeX, e.relativeY);
        }
    }

    void DXRenderer::OnMouseButtonPressed(events::MouseButtonEvent& e)
    {
        if (e.rightButton)
        {
            _isCameraMoving = true;
        }
    }

    void DXRenderer::OnMouseButtonReleased(events::MouseButtonEvent& e)
    {
        if (!e.rightButton)
        {
            _isCameraMoving = false;
        }
    }

    void DXRenderer::OnResize(core::events::ResizeEvent& e)
    {
        if (e.width == 0 && e.height == 0)
        {
            LOG_INFO("Window minimized.");
            _isMinimized = true;
            return; // Do not resize buffers on window minimize (otherwise it'll crash...)
        }
        else
        {
            LOG_INFO("Window resized to {}x{}.", e.width, e.height);
            _isMinimized = false;
        }

        WaitAllFrames();

        Frame* current = _currentFrame;
        do
        {
            current->Resize((uint32_t)e.width, (uint32_t)e.height);
            current = current->Next;
        } while (current != _currentFrame);

        _device->OnResize((uint32_t)e.width, (uint32_t)e.height);
        _cameraComponent->SetSize((uint32_t)e.width, (uint32_t)e.height);
        _cameraComponent->Update();

        CreateShadowMaps();
        SetupRenderPipeline();
    }

    void DXRenderer::OnPipelineChanged()
    {
        WaitAllFrames();
        CreateShadowMaps();
        SetupRenderPipeline();

        LOG_INFO("Render pipeline changed. Rebuilding render graph...");
    }

    void DXRenderer::OnLoadScene(const std::string& filepath)
    {
        WaitAllFrames();

        rg::ITask* task = _currentFrame->AllocateTask(rhi::CommandListType::Graphics, nullptr);
        task->SetName("SceneLoad");

        TaskGPU* uploadTask = _currentFrame->GetTask("SceneLoad");
        LoadContent(uploadTask, filepath);

        rhi::CommandList* commandList = uploadTask->GetCommandList();
        rhi::Fence* fence = uploadTask->GetFence();

        _currentFrame->SetSyncPoint(fence);

        _device->GetGraphicsQueue()->ExecuteCommandLists({ commandList });
        _device->GetGraphicsQueue()->Signal(fence, fence->GetValue());
    }

    void DXRenderer::UpdateSceneBuffers()
    {
        std::shared_ptr<rhi::Buffer> modelBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Model)];
        std::shared_ptr<rhi::Buffer> lightBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Light)];

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshEntities = _scene->FilterNodesByComponent("Mesh");
        std::vector<std::shared_ptr<scene::Entity>> animatedEntities = _scene->FilterNodesByComponent("Armature");

            if (!modelBuffer || (meshEntities.size() > (modelBuffer->GetSize() / sizeof(GPUModelDesc))))
            {
                rhi::BufferDescription modelBufferDesc =
                {
                    .Size = static_cast<uint32_t>(meshEntities.size() * sizeof(GPUModelDesc)),
                    .Stride = sizeof(GPUModelDesc),
                    .Usage = rhi::ResourceUsage::Upload
                };
                modelBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Model)] = _device->CreateBuffer(modelBufferDesc, rhi::ResourceState::GenericRead, "scene_models_buffer");
            }

            if (!lightBuffer || (lightEntities.size() > (lightBuffer->GetSize() / sizeof(GPULightDesc))))
            {
                rhi::BufferDescription lightBufferDesc =
                {
                    .Size = static_cast<uint32_t>(lightEntities.size() * sizeof(GPULightDesc)),
                    .Stride = sizeof(GPULightDesc),
                    .Usage = rhi::ResourceUsage::Upload
                };
                lightBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Light)] = _device->CreateBuffer(lightBufferDesc, rhi::ResourceState::GenericRead, "scene_lights_buffer");
            }

        GPULightDesc* lights = lightBuffer->Map<GPULightDesc>();

        for (size_t lightIndex = 0; lightIndex < lightEntities.size(); ++lightIndex)
        {
            std::shared_ptr<scene::Entity> lightEntity = lightEntities[lightIndex];
            std::shared_ptr<scene::Light> lightComponent = lightEntity->GetComponentAs<scene::Light>("Light");
            std::shared_ptr<scene::Transformation> transformComponent = lightEntity->GetComponentAs<scene::Transformation>("Transformation");

            std::uint32_t shadowMapIndex = -1;

            if (lightComponent->CastShadows)
            {
                if (lightComponent->ShadowMapHandle == InvalidTextureHandle)
                {
                    CreateShadowMap(lightEntity);
                }

                std::shared_ptr<rhi::Texture> shadowMap = TextureManager::Get().GetTexture(lightComponent->ShadowMapHandle);
                shadowMapIndex = ResourceTable::Get().GetBindlessIndex(shadowMap->GetID(), rhi::ResourceViewType::SRV);
            }

            auto views = GetLightViews(lightEntity);
            XMMATRIX proj = XMMatrixIdentity();
            switch (lightComponent->Type)
            {
            case scene::LightType::Spot:
                proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(lightComponent->OuterAngle), 1.0f, 1.0f, lightComponent->Range);
                break;
            case scene::LightType::Point:
                proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(90.0f), 1.0f, 0.5f, lightComponent->Range);
                break;
            }

            for (auto& view : views)
            {
                view *= proj;
            }

            XMVECTOR direction = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
            direction = XMVector3TransformNormal(direction, transformComponent->Transform);

            lights[lightIndex] =
            {
                .Direction = DirectX::XMVector3Normalize(direction),
                .Position = transformComponent->Transform.r[3],
                .Color = lightComponent->Color,

                .Intensity = lightComponent->Intensity,
                .Range = lightComponent->Range,
                .OuterAngle = std::cosf(XMConvertToRadians(lightComponent->OuterAngle * 0.5f)), // TODO: fix later
                .InnerAngle = std::cosf(XMConvertToRadians(lightComponent->InnerAngle * 0.5f)),

                .Type = static_cast<std::uint32_t>(lightComponent->Type),
                .CastShadows = lightComponent->CastShadows ? 1u : 0u,

                .PerspectiveValues = { proj.r[2].m128_f32[2], proj.r[3].m128_f32[2] },
                .ViewProj = views,

                .ShadowMapIndex = shadowMapIndex
            };
        }

        GPUModelDesc* models = modelBuffer->Map<GPUModelDesc>();

        for (size_t index = 0; index < meshEntities.size(); ++index)
        {
            std::shared_ptr<scene::Entity> entity = meshEntities[index];
            std::shared_ptr<scene::Material> materialComponent = entity->GetComponentAs<scene::Material>("Material");
            std::shared_ptr<scene::Transformation> transformComponent = entity->GetComponentAs<scene::Transformation>("Transformation");
            std::shared_ptr<scene::Animation> animationComponent = entity->GetComponentAs<scene::Animation>("Animation");
            std::shared_ptr<scene::Armature> armatureComponent = entity->GetComponentAs<scene::Armature>("Armature");

            entity->SetInstanceID(static_cast<scene::Entity::InstanceID>(index));

            std::uint32_t bonesBufferIndex = -1;

            if (armatureComponent && animationComponent)
            {
                const std::vector<scene::Bone*>& bones = armatureComponent->GetSortedBones();

                if (armatureComponent->GetBoneBufferHandle() == InvalidGeometryHandle)
                {
                    rhi::BufferDescription bonesBufferDesc =
                    {
                        .Size = static_cast<std::uint32_t>(sizeof(DirectX::XMMATRIX) * bones.size()),
                        .Stride = sizeof(DirectX::XMMATRIX),
                        .Usage = rhi::ResourceUsage::Upload
                    };
                    std::shared_ptr<rhi::Buffer> bonesBuffer = _device->CreateBuffer(bonesBufferDesc, rhi::ResourceState::CopyDest, entity->GetName() + "_bones_buffer");

                    armatureComponent->SetBoneBufferHandle(GeometryCacheManager::Get().CacheGeometry(bonesBuffer));
                }

                std::shared_ptr<rhi::Buffer> bonesBuffer = GeometryCacheManager::Get().GetGeometry(armatureComponent->GetBoneBufferHandle());
                DirectX::XMMATRIX* data = bonesBuffer->Map<DirectX::XMMATRIX>();

                for (int i = 0; i < bones.size(); ++i)
                {
                    data[i] = bones[i]->Offset * bones[i]->GlobalTransform;
                }

                ResourceTable::Get().CreateTransientResourceView(bonesBuffer, rhi::ResourceViewType::SRV);
                bonesBufferIndex = ResourceTable::Get().GetBindlessIndex(bonesBuffer->GetID(), rhi::ResourceViewType::SRV);;
            }

            std::uint32_t albedoTextureIndex = -1;
            std::uint32_t emissionTextureIndex = -1;
            std::uint32_t normalMapIndex = -1;
            std::uint32_t metalnessTextureIndex = -1;
            std::uint32_t roughnessTextureIndex = -1;

            if (materialComponent)
            {
                std::shared_ptr<rhi::Texture> albedoTexture = TextureManager::Get().GetTexture(materialComponent->AlbedoTextureHandle);
                std::shared_ptr<rhi::Texture> emissionTexture = TextureManager::Get().GetTexture(materialComponent->EmissionTextureHandle);
                std::shared_ptr<rhi::Texture> normalMapTexture = TextureManager::Get().GetTexture(materialComponent->NormalMapTextureHandle);
                std::shared_ptr<rhi::Texture> metalnessTexture = TextureManager::Get().GetTexture(materialComponent->MetalnessTextureHandle);
                std::shared_ptr<rhi::Texture> roughnessTexture = TextureManager::Get().GetTexture(materialComponent->RoughnessTextureHandle);

                if (albedoTexture)
                {
                    ResourceTable::Get().CreateStaticResourceView(albedoTexture, rhi::ResourceViewType::SRV);
                    albedoTextureIndex = ResourceTable::Get().GetBindlessIndex(albedoTexture->GetID(), rhi::ResourceViewType::SRV);
                }
                if (emissionTexture)
                {
                    ResourceTable::Get().CreateStaticResourceView(emissionTexture, rhi::ResourceViewType::SRV);
                    emissionTextureIndex = ResourceTable::Get().GetBindlessIndex(emissionTexture->GetID(), rhi::ResourceViewType::SRV);
                }
                if (normalMapTexture)
                {
                    ResourceTable::Get().CreateStaticResourceView(normalMapTexture, rhi::ResourceViewType::SRV);
                    normalMapIndex = ResourceTable::Get().GetBindlessIndex(normalMapTexture->GetID(), rhi::ResourceViewType::SRV);
                }
                if (metalnessTexture)
                {
                    ResourceTable::Get().CreateStaticResourceView(metalnessTexture, rhi::ResourceViewType::SRV);
                    metalnessTextureIndex = ResourceTable::Get().GetBindlessIndex(metalnessTexture->GetID(), rhi::ResourceViewType::SRV);
                }
                if (roughnessTexture)
                {
                    ResourceTable::Get().CreateStaticResourceView(roughnessTexture, rhi::ResourceViewType::SRV);
                    roughnessTextureIndex = ResourceTable::Get().GetBindlessIndex(roughnessTexture->GetID(), rhi::ResourceViewType::SRV);
                }
            }

            // TOOD: this is a temporary solution, need to be fixed
            models[index] =
            {
                .Transform = transformComponent->Transform,
                .AlbedoTextureIndex = albedoTextureIndex,
                .EmissionTextureIndex = emissionTextureIndex,
                .NormalMapTextureIndex = normalMapIndex,
                .MetalnessTextureIndex = metalnessTextureIndex,
                .RoughnessTextureIndex = roughnessTextureIndex,

                .EmissionIntensity = materialComponent ? materialComponent->EmissionIntensity : 0.0f,
                .MetallicValue = materialComponent ? materialComponent->MetallicValue : 0.0f,
                .RoughnessValue = materialComponent ? materialComponent->RoughnessValue : 1.0f,
                .AlbedoColor = materialComponent ? materialComponent->AlbedoColor : DirectX::XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f),
                .EmissionColor = materialComponent ? materialComponent->EmissionColor : DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),

                .HasMesh = 1,
                .BonesBufferIndex = bonesBufferIndex
            };
        }

        ResourceTable::Get().CreateTransientResourceView(modelBuffer, rhi::ResourceViewType::SRV);
        ResourceTable::Get().CreateTransientResourceView(lightBuffer, rhi::ResourceViewType::SRV);

        {
            GPUFrameDesc* frameBufferData = _currentFrame->GetBuffer()->Map<GPUFrameDesc>();

            DirectX::XMUINT2 windowSize = _cameraComponent->GetSize();

            frameBufferData[0] =
            {
                .View = _cameraComponent->View(),
                .Projection = _cameraComponent->Projection(),
                .ViewProjection = _cameraComponent->ViewProjection(),

                .InvView = XMMatrixInverse(nullptr, _cameraComponent->View()),
                .InvProjection = XMMatrixInverse(nullptr, _cameraComponent->Projection()),

                .EyePosition = _cameraComponent->Position(),
                .EyeDirection = _cameraComponent->Look(),

                .WindowSize = { windowSize.x, windowSize.y },
                .ReciprocalWindowSize = { 1.0f / windowSize.x, 1.0f / windowSize.y },
                .NearFar = { _cameraComponent->GetNearZ(), _cameraComponent->GetFarZ()},

                .InstancesBufferIndex = ResourceTable::Get().GetBindlessIndex(modelBuffer->GetID(), rhi::ResourceViewType::SRV),
                .LightsBufferIndex = ResourceTable::Get().GetBindlessIndex(lightBuffer->GetID(), rhi::ResourceViewType::SRV),
                .LightsNum = static_cast<std::uint32_t>(lightEntities.size()),

                .DeltaTime = _deltaTime
            };
        }
    }

    void DXRenderer::CreateShadowMap(std::shared_ptr<scene::Entity> light)
    {
        std::shared_ptr<scene::Light> lightComponent = light->GetComponentAs<scene::Light>("Light");

        if (lightComponent && lightComponent->CastShadows)
        {
            auto viewportSize = _cameraComponent->GetSize();

            std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2.0f;
            rhi::TextureDescription shadowMapDesc =
            {
                .Width = size,
                .Height = size,
                .DepthOrArraySize = (lightComponent->Type == scene::LightType::Point) ? uint16_t(6) : uint16_t(1),
                .ClearValue = { .DepthStencil = { 1.0f, 0 } },
                .Format = rhi::Format::D32_FLOAT,
                .Dimension = rhi::TextureDimension::Texture2D,
                .Flags = rhi::ResourceFlags::AllowDepthStencil
            };

            std::shared_ptr<rhi::Texture> shadowMap = _device->CreateTexture(shadowMapDesc, rhi::ResourceState::Common, light->GetName() + "_shadow_map");
            lightComponent->ShadowMapHandle = TextureManager::Get().AddTexture(shadowMap);
            lightComponent->ShadowMapId = shadowMap->GetID();

            ResourceTable::Get().CreateStaticResourceView(shadowMap, rhi::ResourceViewType::DSV);
            ResourceTable::Get().CreateStaticResourceView(shadowMap, rhi::ResourceViewType::SRV);
        }
    }

    void DXRenderer::CreateShadowMaps()
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

        for (const auto& lightEntity : lightEntities)
        {
            CreateShadowMap(lightEntity);
        }
    }

    void DXRenderer::UpdateEntity(std::shared_ptr<scene::Entity> entity)
    {
        entity->UpdateGlobalTransform();

        std::shared_ptr<scene::Armature> armature = entity->GetComponentAs<scene::Armature>("Armature");
        std::shared_ptr<scene::Animation> animation = entity->GetComponentAs<scene::Animation>("Animation");
        std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

        if (armature && animation)
        {
            animation->Update(_deltaTime);

            const auto& transforms = animation->GetBonesTransforms();
            armature->ApplyAnimation(transforms);
            armature->UpdateGlobalTransformations();
        }

        if (mesh)
        {
            UpdateBoundingVolumes(entity);
        }

        for (const auto& child : entity->GetChildrenNodes())
        {
            UpdateEntity(child);
        }
    }

    void DXRenderer::UpdateBoundingVolumes(std::shared_ptr<scene::Entity> entity)
    {
        std::shared_ptr<scene::Armature> armature = entity->GetComponentAs<scene::Armature>("Armature");
        std::shared_ptr<scene::Transformation> transformation = entity->GetComponentAs<scene::Transformation>("Transformation");
        std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

        if (mesh && armature)
        {
            std::vector<scene::OBBVolume> boneOBBs;
            boneOBBs.reserve(armature->GetBones().size());

            for (const auto& bone : armature->GetSortedBones())
            {
                DirectX::XMMATRIX boneOBB = bone->OBB.Bounds;
                boneOBB *= bone->GlobalTransform * transformation->Transform;

                scene::OBBVolume obb;
                obb.Bounds = boneOBB;

                boneOBBs.push_back(obb);
            }

            mesh->GlobalAABB = scene::CombineOBBs(boneOBBs);
        }
        else if (mesh)
        {
            mesh->GlobalAABB = mesh->LocalAABB.Transform(transformation->Transform);
        }
    }

    void DXRenderer::WaitAllFrames()
    {
        Frame* current = _currentFrame;
        do
        {
            current->WaitCPU();
            current->ResetGPU();
            current = current->Next;
        } while (current != _currentFrame);
    }

    void DXRenderer::SetupRenderPipeline()
    {
        _gpuProfiler.UnregisterAllTimers();

        // Render Graph setup
        {
            _renderGraph->Reset();

            // Import IBL textures to render graph

            if (_diffuseIrradianceMap)
            {
                _renderGraph->ImportResource(_diffuseIrradianceMap, "diffuse_irradiance_map");
            }
            if (_preFilteredEnvironmentMap)
            {
                _renderGraph->ImportResource(_preFilteredEnvironmentMap, "prefiltered_environment_map");
            }
            if (_brdfLUT)
            {
                _renderGraph->ImportResource(_brdfLUT, "brdf_lut");
            }

            std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

            for (const auto& lightEntity : lightEntities)
            {
                std::shared_ptr<scene::Light> lightComponent = lightEntity->GetComponentAs<scene::Light>("Light");

                if (lightComponent && lightComponent->CastShadows)
                {
                    _renderGraph->ImportResource(TextureManager::Get().GetTexture(lightComponent->ShadowMapHandle), lightEntity->GetName() + "_shadow_map");
                }
            }

            // Add passes to render graph

            _renderGraph->AddPass(std::make_shared<GeometryPass>(_device, _scene, _cameraComponent.get()));
            //_renderGraph->AddPass(std::make_shared<ShadowCullPass>(_device, _scene, _cameraComponent.get()));
            //_renderGraph->AddPass(std::make_shared<ShadowDrawPass>(_device, _scene, _cameraComponent.get()));
            //_renderGraph->AddPass(std::make_shared<AmbientLightingPass>(_device, _scene, _cameraComponent.get()));
            //if (RenderSettings::UseSSAO())
            //{
            //    _renderGraph->AddPass(std::make_shared<SSAOComputePass>(_device, _scene, _cameraComponent.get()));
            //    _renderGraph->AddPass(std::make_shared<SSAOBlurPass>(_device, _scene, _cameraComponent.get()));
            //    _renderGraph->AddPass(std::make_shared<SSAOApplyPass>(_device, _scene, _cameraComponent.get()));
            //}
            //_renderGraph->AddPass(std::make_shared<LightingPass>(_device, _scene, _cameraComponent.get()));
            //if (auto skyboxNode = _scene->FindNodeByComponentName("Skybox"))
            //{
            //    std::shared_ptr<scene::Skybox> skybox = skyboxNode->GetComponentAs<scene::Skybox>("Skybox");
            //    std::shared_ptr<rhi::Texture> skyboxTexture = TextureManager::Get().GetTexture(skybox->SkydomeTextureHandle);
            //
            //    if (skyboxTexture)
            //    {
            //        _renderGraph->ImportResource(skyboxTexture, "skybox");
            //        _renderGraph->AddPass(std::make_shared<SkyboxPass>(_device, _scene, _cameraComponent.get()));
            //    }
            //}
            //if (RenderSettings::UseBloom())
            //{
            //    _renderGraph->AddPass(std::make_shared<BloomDownsamplePass>(_device, _scene, _cameraComponent.get()));
            //    _renderGraph->AddPass(std::make_shared<BloomUpsamplePass>(_device, _scene, _cameraComponent.get()));
            //    _renderGraph->AddPass(std::make_shared<BloomApplyPass>(_device, _scene, _cameraComponent.get()));
            //}
            //if (RenderSettings::UseFXAA())
            //{
            //    _renderGraph->AddPass(std::make_shared<FXAAPass>(_device, _scene, _cameraComponent.get()));
            //}
            //_renderGraph->AddPass(std::make_shared<LuminanceHistogramPass>(_device, _scene, _cameraComponent.get()));
            //_renderGraph->AddPass(std::make_shared<AverageLuminancePass>(_device, _scene, _cameraComponent.get()));
            //_renderGraph->AddPass(std::make_shared<ToneMappingPass>(_device, _scene, _cameraComponent.get()));
            //if (RenderSettings::RenderDebugVolumes())
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugBoundingVolumePass>(_device, _scene, _cameraComponent.get()));
            //}
            //if (RenderSettings::RenderDebugArmature())
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugArmaturePass>(_device, _scene, _cameraComponent.get()));
            //}
            //if (RenderSettings::DebugView().ShowAlbedo)
            {
                _renderGraph->AddPass(std::make_shared<DebugAlbedoViewPass>(_device, _scene, _cameraComponent.get()));
            }
            //else if (RenderSettings::DebugView().ShowMetalness)
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugMetallicViewPass>(_device, _scene, _cameraComponent.get()));
            //}
            //else if (RenderSettings::DebugView().ShowRoughness)
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugRoughnessViewPass>(_device, _scene, _cameraComponent.get()));
            //}
            //else if (RenderSettings::DebugView().ShowNormals)
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugNormalViewPass>(_device, _scene, _cameraComponent.get()));
            //}
            //else if (RenderSettings::DebugView().ShowSSAO)
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugSSAOViewPass>(_device, _scene, _cameraComponent.get()));
            //}
            //else if (RenderSettings::DebugView().ShowEmission)
            //{
            //    _renderGraph->AddPass(std::make_shared<DebugEmissiveViewPass>(_device, _scene, _cameraComponent.get()));
            //}

            struct PresentPassData
            {
                rg::RGTextureCopySrcId RenderTarget;
            };
            static PresentPassData presentPassData;

            _renderGraph->AddPass<PresentPassData>(_device, "present_pass",
                [&](rg::RenderPassBuilder& builder)
                {
                    presentPassData.RenderTarget = builder.CopySrcTexture("render_target");
                },
                [&](rg::RenderContext& context, rg::ITask* task)
                {
                    rhi::CommandList* commandList = task->GetCommandList();

                    {
                        GPU_SCOPED_EVENT(commandList, "Present Pass", 2);

                        std::shared_ptr<rhi::Texture> target = context.GetTexture(presentPassData.RenderTarget);
                        std::shared_ptr<rhi::Texture> swapChainTexture = this->_device->GetBackBuffer();

                        rhi::TextureBarrier beginBarrier = { swapChainTexture, rhi::ResourceState::Present, rhi::ResourceState::CopyDest };
                        rhi::TextureBarrier endBarrier = { swapChainTexture, rhi::ResourceState::CopyDest, rhi::ResourceState::Present };

                        commandList->TransitionBarriers({ beginBarrier });
                        commandList->CopyTexture(target, swapChainTexture);
                        commandList->TransitionBarriers({ endBarrier });
                    }

                    commandList->Close();
                },
                rg::RenderPassType::Graphics);

            _renderGraph->Compile();
        }
    }
} // namespace render
