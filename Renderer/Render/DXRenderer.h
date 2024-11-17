#pragma once

#include "DXObjects/RootSignature.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "Render/Frame/Frame.h"
#include "Window/IWindowEventListener.h"

#include "GBuffer.h"
#include "Scene/Skybox.h"

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
    virtual void OnResize(Core::Events::ResizeEvent& e) override {}

private:
    void ClearBuffers(TaskGPU& task);
    void GeometryPass(TaskGPU& task);
    void LightingPass(TaskGPU& task);
    void RenderSkybox(TaskGPU& task);
    void RenderGUI(TaskGPU& task);
    void Present(TaskGPU& task);

    HWND _windowHandle;

    Core::GBuffer _gBuffer;

    Core::RootSignature _gPassPipeline;
    Core::RootSignature _deferredPipeline;
    Core::RootSignature _renderPipeline;
    Core::RootSignature _AABBpipeline;
    Core::RootSignature _SkyboxPipeline;

    SceneLayer::Skybox _skybox;

    ComPtr<ID3D12RootSignature> _postFXRootSig;
    ComPtr<ID3D12PipelineState> _postFXPipeState;

    Frame* _currentFrame;

    SceneLayer::Scene _scene;
    SceneLayer::Camera _camera;
    bool _isCameraMoving;
    float _deltaTime;

    bool _contentLoaded;
};