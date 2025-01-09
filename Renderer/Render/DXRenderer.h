#pragma once

#include "GBuffer.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Scene.h"
#include "SceneProcessors/UploadSceneProcessor.h"
#include "Render/Frame/Frame.h"
#include "Render/Passes/IRenderPass.h"
#include "Window/IWindowEventListener.h"

class DXRenderer : public Core::Events::IWindowEventListener
{
public:
    DXRenderer(HWND windowHandle);
    ~DXRenderer();

    virtual bool LoadContent(TaskGPU* loadTask);
    virtual void UnloadContent();

    virtual void OnUpdate(Core::Events::UpdateEvent& e) override;
    virtual void OnRender(Core::Events::RenderEvent& e, Frame& frame) override;
    virtual void OnKeyPressed(Core::Events::KeyEvent& e) override;
    virtual void OnKeyReleased(Core::Events::KeyEvent& e) override {}
    virtual void OnMouseMoved(Core::Events::MouseMoveEvent& e) override;
    virtual void OnMouseButtonPressed(Core::Events::MouseButtonEvent& e) override;
    virtual void OnMouseButtonReleased(Core::Events::MouseButtonEvent& e) override;
    virtual void OnMouseScroll(Core::Events::MouseScrollEvent& e) override {}
    virtual void OnResize(Core::Events::ResizeEvent& e) override;

private:
    HWND _windowHandle;

    Core::GBuffer _gBuffer;
    SceneLayer::Scene _scene;
    std::shared_ptr<SceneLayer::Camera> _cameraComponent;

    UploadSceneProcessor _uploadProcessor;

    std::vector<std::unique_ptr<IRenderPass>> _renderPasses;

    bool _isCameraMoving;
    float _deltaTime;

    bool _renderArmature;
    bool _renderAABB;
    bool _renderSkybox;
    bool _applyFXAA;
    float _timeMiltiplier;

    bool _contentLoaded;
};