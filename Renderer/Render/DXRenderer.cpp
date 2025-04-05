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
#include "Render/Passes/Debug/DebugArmaturePass.h"
#include "Render/Passes/Debug/DebugBoundingVolumePass.h"
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

                    modelDesc->AlbedoTextureIndex    = frameResourceTable.CopyDescriptor(textureManager.GetTexture(material->Albedo).get(), dx12::ResourceViewType::SRV, textureTable);
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

        RECT windowSize;
        GetClientRect(_windowHandle, &windowSize);
        uint32_t windowWidth = windowSize.right - windowSize.left;
        uint32_t windowHeight = windowSize.bottom - windowSize.top;
        
        _renderGraph.Reset();

        uploadTask->SetName("Upload Data");
        dx12::CommandList& commandList = *uploadTask->GetCommandLists().front();

        // Load scene
        {
            _sceneLoader.LoadScene(*uploadTask, filepath, _scene);
        }

        // Camera Setup
        {
            std::shared_ptr<scene::Entity> camera = _scene->FilterNodesByComponent("Camera").front();
            _cameraComponent = camera->GetComponentAs<scene::Camera>("Camera");

            _cameraComponent->SetViewport(scene::Viewport({ windowWidth, windowHeight }));
        }

        {
            std::shared_ptr<scene::Entity> skybox = _scene->FilterNodesByComponent("Skybox").front();
            std::shared_ptr<scene::Skybox> skyboxComponent = skybox->GetComponentAs<scene::Skybox>("Skybox");

            _IBL_DiffuseIrradianceConvolution.Parse("PipelineDescriptions\\IBL_DiffuseIrradianceConvolution.tech");
            _IBL_PreFilterEnvMap.Parse("PipelineDescriptions\\IBL_PreFilterEnvMap.tech");
            _IBL_BRDFGenerateLUT.Parse("PipelineDescriptions\\IBL_BRDFGenerateLUT.tech");

            dx12::ResourceDescription diffuseIrradianceTextureDesc;
            {
                diffuseIrradianceTextureDesc.SetSize({ 128, 128 });
                diffuseIrradianceTextureDesc.SetDepthOrArraySize(6);
                diffuseIrradianceTextureDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
                diffuseIrradianceTextureDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered | dx12::ResourceType::Array);
            }
            _diffuseIrradianceMap = std::make_shared<dx12::Texture>();
            _diffuseIrradianceMap->SetName("DiffuseIrradianceMap");
            _diffuseIrradianceMap->CreateCommitedResource(diffuseIrradianceTextureDesc);

            dx12::ResourceDescription preFilteredEnvTextureDesc;
            {
                preFilteredEnvTextureDesc.SetSize({ 512, 512 });
                preFilteredEnvTextureDesc.SetDepthOrArraySize(6);
                preFilteredEnvTextureDesc.SetMipLevels(6);
                preFilteredEnvTextureDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
                preFilteredEnvTextureDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered | dx12::ResourceType::Array);
            }
            _preFilteredEnvMap = std::make_shared<dx12::Texture>();
            _preFilteredEnvMap->SetName("PreFilteredEnvironmentMap");
            _preFilteredEnvMap->CreateCommitedResource(preFilteredEnvTextureDesc);

            dx12::ResourceDescription brdfLUTDesc;
            {
                brdfLUTDesc.SetSize({ 512, 512 });
                brdfLUTDesc.SetFormat(DXGI_FORMAT_R16G16_FLOAT);
                brdfLUTDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
            }
            _brdfLUT = std::make_shared<dx12::Texture>();
            _brdfLUT->SetName("BRDF_LUT");
            _brdfLUT->CreateCommitedResource(brdfLUTDesc);

            dx12::DescriptorHeapDescription desc;
            {
                desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                desc.SetNumDescriptors(9);
                desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
            }
            _descHeap.Reset();
            _descHeap.Create(desc);

            scene::TextureManager& textureManager = _scene->GetCache().GetTextureManager();

            std::shared_ptr<dx12::Texture> skyboxTexture = textureManager.GetTexture(skyboxComponent->SkydomeTexture);

            dx12::Device::CreateShaderResourceView(skyboxTexture->GetAsSRV(), _descHeap);
            dx12::Device::CreateUnorderedAccessView(_diffuseIrradianceMap->GetAsUAV(), _descHeap);
            dx12::Device::CreateUnorderedAccessView(_brdfLUT->GetAsUAV(), _descHeap);

            for (int i = 0; i < 6; ++i)
            {
                dx12::UnorderedAccessView uav;
                {
                    uav.Owner = _preFilteredEnvMap.get();
                    uav.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

                    uav.Texture2DArray.ArraySize = 6;
                    uav.Texture2DArray.MipSlice = i;
                    uav.Texture2DArray.PlaneSlice = 0;
                    uav.Texture2DArray.FirstArraySlice = 0;
                }
                dx12::Device::CreateUnorderedAccessView(uav, _descHeap);
            }

            textureManager.AddTexture(_diffuseIrradianceMap, dx12::ResourceViewType::SRV);
            textureManager.AddTexture(_diffuseIrradianceMap, dx12::ResourceViewType::UAV);
            textureManager.AddTexture(_preFilteredEnvMap, dx12::ResourceViewType::SRV);
            textureManager.AddTexture(_preFilteredEnvMap, dx12::ResourceViewType::UAV);

            std::vector<dx12::ResourceBarrier> barriers =
            {
                { skyboxTexture.get(),          D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
                { _diffuseIrradianceMap.get(),  D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { _preFilteredEnvMap.get(),     D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
                { _brdfLUT.get(),     D3D12_RESOURCE_STATE_COMMON,    D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
            };
            commandList.TransitionBarriers(barriers);

            commandList.SetPipelineState(_IBL_DiffuseIrradianceConvolution);

            commandList.SetDescriptorHeaps({ _descHeap.GetDXDescriptorHeap().Get() });
            commandList.SetDescriptorTable(0, _descHeap.GetGPUHandleWithOffset(0));
            commandList.SetDescriptorTable(1, _descHeap.GetGPUHandleWithOffset(1));
;
            int xThreadGroups = (uint32_t)std::ceilf(diffuseIrradianceTextureDesc.GetSize().x / 8.0f);
            int yThreadGroups = (uint32_t)std::ceilf(diffuseIrradianceTextureDesc.GetSize().y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups, 6);




            commandList.SetPipelineState(_IBL_BRDFGenerateLUT);

            commandList.SetDescriptorTable(0, _descHeap.GetGPUHandleWithOffset(2));

            xThreadGroups = (uint32_t)std::ceilf(brdfLUTDesc.GetSize().x / 8.0f);
            yThreadGroups = (uint32_t)std::ceilf(brdfLUTDesc.GetSize().y / 8.0f);

            commandList.Dispatch(xThreadGroups, yThreadGroups);




            commandList.SetPipelineState(_IBL_PreFilterEnvMap);

            for (int i = 0; i < 6; ++i)
            {
                //commandList.SetDescriptorHeaps({ _descHeap.GetDXDescriptorHeap().Get() });
                commandList.SetConstant(0, 0.95f);
                commandList.SetDescriptorTable(1, _descHeap.GetGPUHandleWithOffset(0));
                commandList.SetDescriptorTable(2, _descHeap.GetGPUHandleWithOffset(i + 3));

                xThreadGroups = (uint32_t)std::ceilf(preFilteredEnvTextureDesc.GetSize().x / (8.0f * std::pow(2, i)));
                yThreadGroups = (uint32_t)std::ceilf(preFilteredEnvTextureDesc.GetSize().y / (8.0f * std::pow(2, i)));

                commandList.Dispatch(xThreadGroups, yThreadGroups, 6);
            }


            barriers =
            {
                { skyboxTexture.get(),          D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COMMON },
                { _diffuseIrradianceMap.get(),  D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_COMMON },
                { _preFilteredEnvMap.get(),     D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_COMMON },
                { _brdfLUT.get(),     D3D12_RESOURCE_STATE_UNORDERED_ACCESS,          D3D12_RESOURCE_STATE_COMMON }
            };
            commandList.TransitionBarriers(barriers);
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
        DebugInfo::Update(updateEvent);

        _deltaTime = updateEvent.elapsedTime;
        _scene->GetCache().SetDeltaTime(_deltaTime);

        for (const auto& entity : _scene->GetRootNodes())
        {
            UpdateEntity(entity);
        }
    }

    void DXRenderer::OnRender(events::RenderEvent& renderEvent)
    {
        _currentFrame->WaitCPU();
        _currentFrame->ResetGPU();

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
        OutputDebugStringA("Right: ");
        OutputDebugStringA(std::format("{} {} {} {}\n", DirectX::XMVectorGetX(_cameraComponent->Right()), DirectX::XMVectorGetY(_cameraComponent->Right()), DirectX::XMVectorGetZ(_cameraComponent->Right()), DirectX::XMVectorGetW(_cameraComponent->Right())).c_str());
        OutputDebugStringA("Movement: ");
        OutputDebugStringA(std::format("{} {} {} {}\n", DirectX::XMVectorGetX(dir), DirectX::XMVectorGetY(dir), DirectX::XMVectorGetZ(dir), DirectX::XMVectorGetW(dir)).c_str());
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
        _currentFrame->Next->SetSyncPoint(uploadTask->GetFence());

        std::vector<ID3D12CommandList*> frameCommandLists = { commandList.GetDXCommandList().Get() };
        uploadTask->GetCommandQueue()->ExecuteCommandLists(1, frameCommandLists.data());
        uploadTask->GetCommandQueue()->Signal(uploadTask->GetDXFence(), uploadTask->GetFenceValue());

        WaitAllFrames();
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
            _renderGraph.ImportResource(_preFilteredEnvMap);

            _renderGraph.AddPass(std::make_shared<GeometryPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowClearPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowCullPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowDrawPass>(_scene, _cameraComponent.get()));
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
