#include "RendererPCH.h"

#include "DXRenderer.h"

#include "CommandList.h"

#include "Events/KeyEvent.h"
#include "Events/MouseButtonEvent.h"
#include "Events/MouseMoveEvent.h"
#include "Events/RenderEvent.h"
#include "Events/UpdateEvent.h"

#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Entity.h"
#include "Utility/DebugInfo.h"

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

namespace render
{
    DXRenderer::DXRenderer(HWND windowHandle)
        : _windowHandle(windowHandle)
        , _currentFrame(nullptr)
        , _contentLoaded(false)
        , _isCameraMoving(false)
        , _deltaTime(0.0f)
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

    bool DXRenderer::LoadContent(TaskGPU* loadTask)
    {
        render::DrawHelper::Init();

        RECT windowSize;
        GetClientRect(_windowHandle, &windowSize);
        uint32_t windowWidth = windowSize.right - windowSize.left;
        uint32_t windowHeight = windowSize.bottom - windowSize.top;

        // Camera Setup
        std::shared_ptr<scene::Entity> cameraEntity = std::make_shared<scene::Entity>(&_scene->GetCache());
        {
            cameraEntity->SetName("Camera");

            XMVECTOR pos = XMVectorSet(15.0f, 25.0f, 35.0f, 1.0f);
            XMVECTOR target = XMVectorSet(-5.0f, 18.0f, -5.0f, 1.0f);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            std::shared_ptr<scene::Camera> cameraComponent = std::make_shared<scene::Camera>();
            cameraComponent->LookAt(pos, target, up);
            cameraComponent->SetViewport(scene::Viewport({ windowWidth, windowHeight }));
            cameraComponent->SetLens(60.0f, 0.1f, 1000.0f);
            cameraComponent->Speed = 70.0f;

            std::shared_ptr<scene::Transformation> transformComponent = std::make_shared<scene::Transformation>();
            transformComponent->Transform = cameraComponent->View();

            cameraEntity->AddComponent(cameraComponent);
            cameraEntity->AddComponent(transformComponent);

            _cameraComponent = cameraComponent;
        }

        // Load scene
        {
            loadTask->SetName("Upload Data");
            dx12::CommandList& commandList = *loadTask->GetCommandLists().front();

            _scene = std::make_shared<scene::Scene>();
            _scene->LoadScene("Dragon\\DragonScene.scene", commandList);
            _uploadProcessor.Process(*_scene, commandList, nullptr);

            _scene->AddRootNode(cameraEntity);

            commandList.Close();
        }

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

        _scene->GetCache().SetTime(updateEvent.totalTime);
        _scene->GetCache().SetDeltaTime(updateEvent.elapsedTime);

        _deltaTime = updateEvent.elapsedTime;

        std::function<void(std::shared_ptr<scene::Entity>)> updateEntity = [&](std::shared_ptr<scene::Entity> entity)
            {
                entity->UpdateGlobalTransform();

                scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
                scene::Animation* animation = entity->GetComponentAs<scene::Animation>("Animation");

                if (armature && animation)
                {
                    const auto& transforms = animation->GetBonesTransforms(updateEvent.totalTime);
                    armature->ApplyAnimation(transforms);
                    armature->UpdateGlobalTransformations();
                }

                for (const auto& child : entity->GetChildrenNodes())
                {
                    updateEntity(child);
                }
            };

        for (const auto& entity : _scene->GetRootNodes())
        {
            updateEntity(entity);
        }
    }

    void DXRenderer::OnRender(events::RenderEvent& renderEvent)
    {
        _currentFrame->WaitCPU();
        _currentFrame->ResetGPU();

        _renderGraph.Execute(*_currentFrame);

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
        auto cameraEntity = _scene->FindNodeByComponentName("Camera");
        ASSERT(cameraEntity.get(), "No camera on the scene");
        scene::Camera* camera = cameraEntity->GetComponentAs<scene::Camera>("Camera");

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
        Frame* current = _currentFrame;
        do
        {
            current->WaitCPU();
            current->ResetGPU();
            current = current->Next;
        } while (current != _currentFrame);

        DirectX::XMUINT2 windowSize = { (uint32_t)e.width, (uint32_t)e.height };

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

    void DXRenderer::SetupRenderPipeline()
    {
        // Render Graph setup
        {
            _renderGraph.Reset();

            _renderGraph.AddPass(std::make_shared<GeometryPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowClearPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowCullPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ShadowDrawPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<LightingPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<SkyboxPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<LuminanceHistogramPass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<AverageLuminancePass>(_scene, _cameraComponent.get()));
            _renderGraph.AddPass(std::make_shared<ToneMappingPass>(_scene, _cameraComponent.get()));
            //_renderGraph.AddPass(std::make_shared<FXAAPass>(_scene, _cameraComponent.get()));
            //_renderGraph.AddPass(std::make_shared<DebugBoundingVolumePass>(_scene, _cameraComponent.get()));
            //_renderGraph.AddPass(std::make_shared<DebugArmaturePass>(_scene, _cameraComponent.get()));

            _renderGraph.Compile();
        }
    }
} // namespace render
