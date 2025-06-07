#include "RendererPCH.h"

#include "DXRenderer.h"

#include "CommandList.h"
#include "Texture.h"
#include "ResourceBarrier.h"

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
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Entity.h"
#include "Utility/DebugInfo.h"

#include "Render/Helpers/GPUStructs.h"
#include "Render/Helpers/RenderHelpers.h"

#include "Render/RenderSettings.h"
#include "Render/Frame/TaskGPU.h"
#include "Render/Passes/AmbientOcclusion/SSAOApplyPass.h"
#include "Render/Passes/AmbientOcclusion/SSAOBlurPass.h"
#include "Render/Passes/AmbientOcclusion/SSAOComputePass.h"
#include "Render/Passes/Debug/DebugArmaturePass.h"
#include "Render/Passes/Debug/DebugBoundingVolumePass.h"
#include "Render/Passes/AmbientLightingPass.h"
#include "Render/Passes/FXAAPass.h"
#include "Render/Passes/GeometryPass.h"
#include "Render/Passes/LightingPass.h"
#include "Render/Passes/PFX/AverageLuminancePass.h"
#include "Render/Passes/PFX/LuminanceHistogramPass.h"
#include "Render/Passes/PFX/ToneMappingPass.h"
#include "Render/Passes/ShadowClearPass.h"
#include "Render/Passes/ShadowCullPass.h"
#include "Render/Passes/ShadowDrawPass.h"
#include "Render/Passes/SkyboxPass.h"
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

    void SetupEntity(std::shared_ptr<scene::Scene> scene, std::shared_ptr<scene::Entity> entity, CacheGPU& frameCache, dx12::ResourceTable& frameResourceTable)
    {
        if (std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh"))
        {
            scene::Transformation transform = entity->GetGlobalTransform();
            std::shared_ptr<scene::Armature> armature = entity->GetComponentAs<scene::Armature>("Armature");
            std::shared_ptr<scene::Material> material = entity->GetComponentAs<scene::Material>("Material");

            CacheGPU::DataHandle modelDescHandle = frameCache.RequestPlacement(entity->GetName(), sizeof(GPUModelDesc));
            GPUModelDesc* modelDesc = (GPUModelDesc*)modelDescHandle.DataCPU;
            {
                modelDesc->Transform = transform.Transform;

                if (mesh)
                {
                    modelDesc->HasMesh = 1;
                }

                if (material)
                {
                    scene::TextureManager& textureManager = scene->GetCache().GetTextureManager();
                    dx12::ResourceTable& textureTable = textureManager.GetTextureTable();

                    modelDesc->AlbedoTextureIndex = frameResourceTable.CopyDescriptor(textureManager.GetTexture(material->Albedo).get(), dx12::ResourceViewType::SRV, textureTable);
                    modelDesc->NormalMapTextureIndex = frameResourceTable.CopyDescriptor(textureManager.GetTexture(material->NormalMap).get(), dx12::ResourceViewType::SRV, textureTable);
                    modelDesc->MetalnessTextureIndex = frameResourceTable.CopyDescriptor(textureManager.GetTexture(material->Metalness).get(), dx12::ResourceViewType::SRV, textureTable);
                    modelDesc->RoughnessTextureIndex = frameResourceTable.CopyDescriptor(textureManager.GetTexture(material->Roughness).get(), dx12::ResourceViewType::SRV, textureTable);
                }

                if (armature)
                {
                    modelDesc->UseSkinning = 1;
                }
            }

            // Update and setup animantion
            if (armature)
            {
                const std::vector<scene::Bone*>& bones = armature->GetSortedBones();

                CacheGPU::DataHandle bonesDescHandle = frameCache.RequestPlacement(entity->GetName() + "_bones", sizeof(DirectX::XMMATRIX) * bones.size());
                DirectX::XMMATRIX* data = (DirectX::XMMATRIX*)bonesDescHandle.DataCPU;

                for (int i = 0; i < bones.size(); ++i)
                {
                    data[i] = bones[i]->Offset * bones[i]->GlobalTransform;
                }
            }
        }

        for (std::shared_ptr<scene::Entity>& child : entity->GetChildrenNodes())
        {
            SetupEntity(scene, child, frameCache, frameResourceTable);
        }
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
            // Load scene
            if (std::filesystem::exists(std::filesystem::path(filepath)))
            {
                _sceneLoader.LoadScene(*uploadTask, filepath, _scene);
            }
            else
            {
                LOG_WARNING(false, std::format("Failed to load scene \'{}\'", filepath));
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

            // Generate textures for IBL
            _diffuseIrradianceMap       = _sceneLoader.GenerateEnvironmentDiffuseIrradianceMap(commandList, _scene);
            _brdfLUT                    = _sceneLoader.GenerateEnvironmentBRDFLookUpTexture(commandList, _scene);
            _preFilteredEnvironmentMap  = _sceneLoader.GeneratePreFilteredEnvironmentMap(commandList, _scene);
        }

        commandList.Close();

        SetupRenderPipeline();

        _contentLoaded = true;
        return _contentLoaded;
    }

    void DXRenderer::UnloadContent()
    {
        render::DrawHelper::Destroy();

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
        _scene->GetCache().SetDeltaTime(_deltaTime);

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

        DebugInfo::BeginRender(renderEvent);

        if (_isMinimized)
        {
            return;
        }

        _renderGraph.SetFrame(*_currentFrame);
        UploadSceneCache(_renderGraph.GetCache(), _renderGraph.GetResourceTable());

        _renderGraph.Execute();

        std::shared_ptr<dx12::Resource> target = _renderGraph.ExportResource("Target");

        // Present
        {
            TaskGPU* task = _currentFrame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr);
            task->SetName("present");
            task->AddDependency("GUI Pass"); // TODO: remove hardcoded render dependency !!!

            dx12::CommandList& commandList = *task->GetCommandLists().front();
            commandList.SetName("present");

            PIXBeginEvent(commandList.GetDXCommandList().Get(), 6, "Present");
            {
                dx12::Resource& swapChainTexture = *dx12::Device::GetBackBuffer();

                commandList.TransitionBarrier(swapChainTexture, D3D12_RESOURCE_STATE_COPY_DEST);
                commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COPY_SOURCE);

                commandList.CopyResource(*target, swapChainTexture);

                commandList.TransitionBarrier(swapChainTexture, D3D12_RESOURCE_STATE_PRESENT);
                commandList.TransitionBarrier(*target, D3D12_RESOURCE_STATE_COMMON);
            }
            PIXEndEvent(commandList.GetDXCommandList().Get());

            commandList.Close();
        }

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
            _isMinimized = true;
            return; // Do not resize buffers on window minimize (otherwise it'll crash...)
        }
        else
        {
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

        SetupRenderPipeline();
    }

    void DXRenderer::OnPipelineChanged()
    {
        WaitAllFrames();
        SetupRenderPipeline();
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

    void DXRenderer::UpdateEntity(std::shared_ptr<scene::Entity> entity)
    {
        entity->UpdateGlobalTransform();

        std::shared_ptr<scene::Armature> armature = entity->GetComponentAs<scene::Armature>("Armature");
        std::shared_ptr<scene::Animation> animation = entity->GetComponentAs<scene::Animation>("Animation");
        std::shared_ptr<scene::Transformation> transformation = entity->GetComponentAs<scene::Transformation>("Transformation");
        std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

        if (armature && animation)
        {
            const auto& transforms = animation->GetBonesTransforms(_deltaTime);
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
                boneOBB *= bone->Offset * bone->GlobalTransform * transformation->Transform;

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
        // Render Graph setup
        {
            _renderGraph.Reset();

            _renderGraph.ImportResource(_diffuseIrradianceMap);
            _renderGraph.ImportResource(_brdfLUT);
            _renderGraph.ImportResource(_preFilteredEnvironmentMap);

            _renderGraph.AddPass(std::make_shared<GeometryPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowClearPass>(_scene, _cameraComponent.get()));
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

            _renderGraph.Compile();
        }
    }

    void DXRenderer::UploadSceneCache(CacheGPU& cache, dx12::ResourceTable& table)
    {
        cache.Clear();

        helpers::SetupSceneDataGPU(*_scene, &cache);
        helpers::SetupLightDataGPU(*_scene, &cache, table);

        for (std::shared_ptr<scene::Entity> entity : _scene->GetRootNodes())
        {
            SetupEntity(_scene, entity, cache, table);
        }
    }
} // namespace render
