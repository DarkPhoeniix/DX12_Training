#pragma once

#include "DXObjects/RootSignature.h"
#include "Scene/Nodes/Camera/Camera.h"
#include "Scene/Scene.h"
#include "Render/Frame.h"
#include "Render/IRenderer.h"

class ForwardShadingRenderer : public IRenderer
{
public:
    ForwardShadingRenderer(HWND windowHandle);
    ~ForwardShadingRenderer();

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
    virtual void OnResize(Core::Events::ResizeEvent& e) override {}

private:
    Core::RootSignature _renderPipeline;
    Core::RootSignature _AABBpipeline;

    SceneLayer::Scene _scene;

    SceneLayer::Camera _camera;
    bool _isCameraMoving;
    float _deltaTime;
};