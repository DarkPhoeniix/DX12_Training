#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "DXObjects/Texture.h"
#include "DXObjects/StatisticsQuery.h"
#include "Scene/Nodes/Camera/Camera.h"
#include "Scene/Scene.h"
#include "Scene/Nodes/Light/DirectionalLight.h"
#include "Render/Frame.h"
#include "Window/IWindowEventListener.h"

#include "GBuffer.h"

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
    HWND _windowHandle;

    Core::GBuffer _gBuffer;

    Core::RootSignature _gPassPipeline;
    Core::RootSignature _deferredPipeline;
    Core::RootSignature _renderPipeline;
    Core::RootSignature _AABBpipeline;

    ComPtr<ID3D12RootSignature> _postFXRootSig;
    ComPtr<ID3D12PipelineState> _postFXPipeState;

    std::shared_ptr<Core::Resource> _ambient;

    SceneLayer::Scene _scene;
    SceneLayer::Camera _camera;
    bool _isCameraMoving;
    float _deltaTime;

    bool _contentLoaded;
};