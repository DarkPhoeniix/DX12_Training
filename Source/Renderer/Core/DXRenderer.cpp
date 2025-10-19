#include "RendererPCH.h"

#include "DXRenderer.h"

#include "CommandList.h"

#include "IGPUCrashTracker.h"

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
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Entity.h"
#include "Utility/DebugInfo.h"

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
#include "Render/Passes/PresentPass.h"
#include "Render/Helpers/DrawHelpers.h"

using namespace DirectX;
using namespace core;

namespace
{
    constexpr char DEFAULT_SCENE_PATH[] = "Sponza\\Sponza.scene";

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
}

namespace render
{
    DXRenderer::DXRenderer(HWND windowHandle)
        : _windowHandle(windowHandle)
        , _currentFrame(nullptr)
        , _contentLoaded(false)
        , _isMinimized(false)
        , _isCameraMoving(false)
        , _deltaTime(0.0f)
        , _scene(std::make_shared<scene::Scene>())
    {
        DescriptorHeapManager::Create(2048, 128, 4096, 1024);
        ResourceTable::Create();
		TextureManager::Create();
        GeometryCacheManager::Create();

		_renderGraph.Init(ResourceTable::Get(), TextureManager::Get());
		_sceneLoader.Init(ResourceTable::Get(), TextureManager::Get());
    }

    DXRenderer::~DXRenderer()
    {
    }

    rg::RenderGraph& DXRenderer::GetRenderGraph()
    {
        return _renderGraph;
    }

    std::shared_ptr<scene::Scene> DXRenderer::GetCurrentScene()
    {
        return _scene;
    }

    bool DXRenderer::LoadContent(TaskGPU* uploadTask, const std::string& filepath)
    {
        render::DrawHelper::Init();

        uploadTask->SetName("Upload Data");
        dx12::CommandList& commandList = *uploadTask->GetCommandLists().front();

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

            std::shared_ptr<scene::Entity> camera = _scene->FilterNodesByComponent("Camera").front();
            _cameraComponent = camera->GetComponentAs<scene::Camera>("Camera");
            _cameraComponent->SetViewport(scene::Viewport({ windowWidth, windowHeight }));

            dx12::ResourceDescription frameBufferDesc;
            {
                frameBufferDesc.SetSize({ static_cast<std::uint32_t>(sizeof(GPUFrameDesc)), 1 });
                frameBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
            }
            for (size_t i = 0; i < dx12::BACK_BUFFER_COUNT; ++i)
            {
                std::shared_ptr<dx12::Resource> frameBuffer = ResourceFactory::Create(std::format("frame_buffer_{}", i), frameBufferDesc);
                frameBuffer->CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_DEST);

                _currentFrame->SetBuffer(frameBuffer);

                _currentFrame = _currentFrame->Next;
            }

            // Generate textures for IBL
            _diffuseIrradianceMap = _sceneLoader.GenerateEnvironmentDiffuseIrradianceMap(commandList, _scene);
            _brdfLUT = _sceneLoader.GenerateEnvironmentBRDFLookUpTexture(commandList, _scene);
            _preFilteredEnvironmentMap = _sceneLoader.GeneratePreFilteredEnvironmentMap(commandList, _scene);
        }

        commandList.Close();

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

        // Clear marker map for current frame before execution
        std::shared_ptr<tracking::IGPUCrashTracker> crashTracker = dx12::Device::GetCrashTracker();
        crashTracker->AdvanceFrame();
        crashTracker->ResetMarkerMapForCurrentFrame();

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

        UpdateSceneBuffers();

        DebugInfo::BeginRender(renderEvent);

        if (_isMinimized)
        {
            return;
        }

        _renderGraph.SetFrame(*_currentFrame);
        _renderGraph.Execute();

        DebugInfo::EndRender();
    }

    void DXRenderer::OnKeyPressed(events::KeyEvent& e)
    {
        XMVECTOR dir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        if (e.keyCode == DIKeyCode::DIK_W)
        {
            dir += _cameraComponent->Look() * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_S)
        {
            dir -= _cameraComponent->Look() * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_D)
        {
            dir += _cameraComponent->Right() * _deltaTime;
        }
        if (e.keyCode == DIKeyCode::DIK_A)
        {
            dir -= _cameraComponent->Right() * _deltaTime;
        }
        _cameraComponent->Update(dir);

        switch (e.keyCode)
        {
        case DIKeyCode::DIK_ESCAPE:
            ::SendMessage(_windowHandle, WM_DESTROY, 0, 0);
            break;
        }
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

        DirectX::XMUINT2 windowSize = { (uint32_t)e.width, (uint32_t)e.height };

        Frame* current = _currentFrame;
        do
        {
            current->Resize(windowSize);
            current = current->Next;
        } while (current != _currentFrame);

        dx12::Device::OnResize(windowSize);
        _cameraComponent->GetViewport().SetSize(windowSize);
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

        TaskGPU* uploadTask = _currentFrame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE, nullptr);
        LoadContent(uploadTask, filepath);

        dx12::CommandList& commandList = *uploadTask->GetCommandLists().front();
        _currentFrame->SetSyncPoint(uploadTask->GetFence());

        std::vector<ID3D12CommandList*> frameCommandLists = { commandList.GetDXCommandList().Get() };
        uploadTask->GetCommandQueue()->ExecuteCommandLists(1, frameCommandLists.data());
        uploadTask->GetCommandQueue()->Signal(uploadTask->GetDXFence(), uploadTask->GetFenceValue());
    }

    void DXRenderer::UpdateSceneBuffers()
    {
        std::shared_ptr<dx12::Resource> modelBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Model)];
        std::shared_ptr<dx12::Resource> lightBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Light)];

        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");
        std::vector<std::shared_ptr<scene::Entity>> meshEntities = _scene->FilterNodesByComponent("Mesh");
        std::vector<std::shared_ptr<scene::Entity>> animatedEntities = _scene->FilterNodesByComponent("Armature");

        {
            if (!modelBuffer || (meshEntities.size() > (modelBuffer->GetResourceDescription().GetSize().x / sizeof(GPUModelDesc))))
            {
                dx12::ResourceDescription modelBufferDesc;
                modelBufferDesc.SetSize({ static_cast<uint32_t>(meshEntities.size() * sizeof(GPUModelDesc)), 1 });
                modelBufferDesc.SetStride(sizeof(GPUModelDesc));
                modelBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
                modelBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Model)] = ResourceFactory::Create("Scene models buffer", modelBufferDesc);
                modelBuffer->CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_DEST);
            }

            if (!lightBuffer || (lightEntities.size() > (lightBuffer->GetResourceDescription().GetSize().x / sizeof(GPULightDesc))))
            {
                dx12::ResourceDescription lightBufferDesc;
                lightBufferDesc.SetSize({ static_cast<uint32_t>(lightEntities.size() * sizeof(GPULightDesc)), 1 });
                lightBufferDesc.SetStride(sizeof(GPULightDesc));
                lightBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
                lightBuffer = _sceneBuffers[static_cast<size_t>(SceneBufferType::Light)] = ResourceFactory::Create("Scene lights buffer", lightBufferDesc);
                lightBuffer->CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_DEST);
            }
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

                std::shared_ptr<dx12::Resource> shadowMap = TextureManager::Get().GetTexture(lightComponent->ShadowMapHandle);
                DescriptorHandle shadowMapHandle = ResourceTable::Get().GetStaticResourceHandle(shadowMap->GetAsSRV());
                shadowMapIndex = shadowMapHandle.Index;
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

            lights[lightIndex] =
            {
                .Direction = DirectX::XMVector3Normalize(lightComponent->Direction),
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

            std::uint32_t bonesBufferIndex = -1;

            if (armatureComponent && animationComponent)
            {
                const std::vector<scene::Bone*>& bones = armatureComponent->GetSortedBones();

                if (armatureComponent->GetBoneBufferHandle() == InvalidGeometryHandle)
                {
                    dx12::ResourceDescription bonesBufferDesc;
                    bonesBufferDesc.SetSize({ static_cast<std::uint32_t>(sizeof(DirectX::XMMATRIX) * bones.size()), 1 });
                    bonesBufferDesc.SetStride(sizeof(DirectX::XMMATRIX));
                    bonesBufferDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
                    std::shared_ptr<dx12::Resource> bonesBuffer = ResourceFactory::Create(entity->GetName() + "_bones_buffer", bonesBufferDesc);
                    bonesBuffer->CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_DEST);

                    armatureComponent->SetBoneBufferHandle(GeometryCacheManager::Get().CacheGeometry(bonesBuffer));
                }

                std::shared_ptr<dx12::Resource> bonesBuffer = GeometryCacheManager::Get().GetGeometry(armatureComponent->GetBoneBufferHandle());
                DirectX::XMMATRIX* data = bonesBuffer->Map<DirectX::XMMATRIX>();

                for (int i = 0; i < bones.size(); ++i)
                {
                    data[i] = bones[i]->Offset * bones[i]->GlobalTransform;
                }

                DescriptorHandle bonesBufferHandle = ResourceTable::Get().AddTransientResourceView(bonesBuffer->GetAsSRV());
                bonesBufferIndex = bonesBufferHandle.Index;
            }

            std::uint32_t albedoTextureIndex = -1;
            std::uint32_t emissionTextureIndex = -1;
            std::uint32_t normalMapIndex = -1;
            std::uint32_t metalnessTextureIndex = -1;
            std::uint32_t roughnessTextureIndex = -1;

            if (materialComponent)
            {
                std::shared_ptr<dx12::Resource> albedoTexture = TextureManager::Get().GetTexture(materialComponent->AlbedoTextureHandle);
                std::shared_ptr<dx12::Resource> emissionTexture = TextureManager::Get().GetTexture(materialComponent->EmissionTextureHandle);
                std::shared_ptr<dx12::Resource> normalMapTexture = TextureManager::Get().GetTexture(materialComponent->NormalMapTextureHandle);
                std::shared_ptr<dx12::Resource> metalnessTexture = TextureManager::Get().GetTexture(materialComponent->MetalnessTextureHandle);
                std::shared_ptr<dx12::Resource> roughnessTexture = TextureManager::Get().GetTexture(materialComponent->RoughnessTextureHandle);

                if (albedoTexture)
                {
                    albedoTextureIndex = ResourceTable::Get().GetStaticResourceHandle(albedoTexture->GetAsSRV()).Index;
                }
                if (emissionTexture)
                {
                    emissionTextureIndex = ResourceTable::Get().GetStaticResourceHandle(emissionTexture->GetAsSRV()).Index;
                }
                if (normalMapTexture)
                {
                    normalMapIndex = ResourceTable::Get().GetStaticResourceHandle(normalMapTexture->GetAsSRV()).Index;
                }
                if (metalnessTexture)
                {
                    metalnessTextureIndex = ResourceTable::Get().GetStaticResourceHandle(metalnessTexture->GetAsSRV()).Index;
                }
                if (roughnessTexture)
                {
                    roughnessTextureIndex = ResourceTable::Get().GetStaticResourceHandle(roughnessTexture->GetAsSRV()).Index;
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

        DescriptorHandle modelBufferHandle = ResourceTable::Get().AddTransientResourceView(modelBuffer->GetAsSRV());
        DescriptorHandle lightBufferHandle = ResourceTable::Get().AddTransientResourceView(lightBuffer->GetAsSRV());

        {
            GPUFrameDesc* frameBufferData = _currentFrame->GetBuffer()->Map<GPUFrameDesc>();

            DirectX::XMUINT2 windowSize = _cameraComponent->GetViewport().GetSize();

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
                .NearFar = { _cameraComponent->NearZ, _cameraComponent->FarZ },

                .InstancesBufferIndex = modelBufferHandle.Index,
                .LightsBufferIndex = lightBufferHandle.Index,
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
            auto viewportSize = _cameraComponent->GetViewport().GetSize();

            dx12::ResourceDescription shadowMapDesc;
            {
                std::uint32_t size = std::max(viewportSize.x, viewportSize.y) / 2.0f;

                D3D12_CLEAR_VALUE clearValue;
                clearValue.Format = DXGI_FORMAT_D32_FLOAT;
                clearValue.DepthStencil.Depth = 1;
                clearValue.DepthStencil.Stencil = 0;

                shadowMapDesc.SetSize({ size, size });
                shadowMapDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
                shadowMapDesc.SetClearValue(clearValue);
                switch (lightComponent->Type)
                {
                case scene::LightType::Spot:
                    shadowMapDesc.SetDepthOrArraySize(1);
                    break;
                case scene::LightType::Point:
                    shadowMapDesc.SetDepthOrArraySize(6);
                    break;
                }
                shadowMapDesc.SetResourceType(dx12::ResourceType::DepthStencil | dx12::ResourceType::Texture);
            }

            std::shared_ptr<dx12::Resource> shadowMap = ResourceFactory::Create(light->GetName() + "_shadow_map", shadowMapDesc);
            shadowMap->CreateCommitedResource();

            lightComponent->ShadowMapHandle = TextureManager::Get().AddTexture(shadowMap);
        }
    }

    void DXRenderer::CreateShadowMaps()
    {
        std::vector<std::shared_ptr<scene::Entity>> lightEntities = _scene->FilterNodesByComponent("Light");

        auto viewportSize = _cameraComponent->GetViewport().GetSize();

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
        // Render Graph setup5
        {
            _renderGraph.Reset();

            _renderGraph.SetFrame(*_currentFrame);

            _renderGraph.ImportResource(_diffuseIrradianceMap);
            _renderGraph.ImportResource(_brdfLUT);
            _renderGraph.ImportResource(_preFilteredEnvironmentMap);

            _renderGraph.AddPass(std::make_shared<GeometryPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowCullPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowDrawPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<AmbientLightingPass>(_scene, _cameraComponent.get()));
            if (RenderSettings::UseSSAO())
            {
                _renderGraph.AddPass(std::make_shared<SSAOComputePass>(_scene, _cameraComponent.get()));
                _renderGraph.AddPass(std::make_shared<SSAOBlurPass>(_scene, _cameraComponent.get()));
                _renderGraph.AddPass(std::make_shared<SSAOApplyPass>(_scene, _cameraComponent.get()));
            }
            _renderGraph.AddPass(std::make_shared<LightingPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<SkyboxPass>(_scene, _cameraComponent.get()));
            if (RenderSettings::UseBloom())
            {
                _renderGraph.AddPass(std::make_shared<BloomDownsamplePass>(_scene, _cameraComponent.get()));
                _renderGraph.AddPass(std::make_shared<BloomUpsamplePass>(_scene, _cameraComponent.get()));
                _renderGraph.AddPass(std::make_shared<BloomApplyPass>(_scene, _cameraComponent.get()));
            }
            if (RenderSettings::UseFXAA())
            {
                _renderGraph.AddPass(std::make_shared<FXAAPass>(_scene, _cameraComponent.get()));
            }
            _renderGraph.AddPass(std::make_shared<LuminanceHistogramPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<AverageLuminancePass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ToneMappingPass>(_scene, _cameraComponent.get()));
            if (RenderSettings::RenderDebugVolumes())
            {
                _renderGraph.AddPass(std::make_shared<DebugBoundingVolumePass>(_scene, _cameraComponent.get()));
            }
            if (RenderSettings::RenderDebugArmature())
            {
                _renderGraph.AddPass(std::make_shared<DebugArmaturePass>(_scene, _cameraComponent.get()));
            }
            if (RenderSettings::DebugView().ShowAlbedo)
            {
                _renderGraph.AddPass(std::make_shared<DebugAlbedoViewPass>(_scene, _cameraComponent.get()));
            }
            else if (RenderSettings::DebugView().ShowMetalness)
            {
                _renderGraph.AddPass(std::make_shared<DebugMetallicViewPass>(_scene, _cameraComponent.get()));
            }
            else if (RenderSettings::DebugView().ShowRoughness)
            {
                _renderGraph.AddPass(std::make_shared<DebugRoughnessViewPass>(_scene, _cameraComponent.get()));
            }
            else if (RenderSettings::DebugView().ShowNormals)
            {
                _renderGraph.AddPass(std::make_shared<DebugNormalViewPass>(_scene, _cameraComponent.get()));
            }
            else if (RenderSettings::DebugView().ShowSSAO)
            {
                _renderGraph.AddPass(std::make_shared<DebugSSAOViewPass>(_scene, _cameraComponent.get()));
            }
            else if (RenderSettings::DebugView().ShowEmission)
            {
                _renderGraph.AddPass(std::make_shared<DebugEmissiveViewPass>(_scene, _cameraComponent.get()));
            }
            _renderGraph.AddPass(std::make_shared<PresentPass>(_scene, _cameraComponent.get()));

            _renderGraph.Compile();
        }
    }
} // namespace render
