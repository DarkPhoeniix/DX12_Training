#pragma once

#include "GBuffer.h"
#include "PipelineState.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "SceneProcessors/DrawSceneProcessor.h"
#include "SceneProcessors/SetupCachedDataProcessor.h"
#include "SceneProcessors/UploadSceneProcessor.h"
#include "Render/Frame/Frame.h"
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
    void ClearBuffers(TaskGPU& task);
    void GeometryPass(TaskGPU& task);
    void LightingPass(TaskGPU& task);
    void RenderSkybox(TaskGPU& task);
    void RenderArmature(TaskGPU& task);
    void RenderFXAA(TaskGPU& task);
    void RenderAABB(TaskGPU& task);
    void RenderGUI(TaskGPU& task);
    void Present(TaskGPU& task);

    HWND _windowHandle;

    Core::GBuffer _gBuffer;

    dx12::Resource _fxaaRTT;

    dx12::PipelineState _gPassPipeline;
    dx12::PipelineState _deferredPipeline;
    dx12::PipelineState _AABBpipeline;
    dx12::PipelineState _OBBpipeline;
    dx12::PipelineState _SkyboxPipeline;
    dx12::PipelineState _FXAAPipeline;
    dx12::PipelineState _ArmatureDebugPipeline;

    UploadSceneProcessor _uploadProcessor;
    SetupCachedDataProcessor _cachedDataProcessor;
    DrawSceneProcessor _drawProcessor;

    Frame* _currentFrame;

    SceneLayer::Scene _scene;
    SceneLayer::Camera _camera;
    bool _isCameraMoving;
    float _deltaTime;

    bool _renderArmature;
    bool _renderAABB;
    bool _renderSkybox;
    bool _applyFXAA;
    float _timeMiltiplier;

    bool _contentLoaded;
};