#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "Render/Frame.h"
#include "Render/FencePool.h"
#include "Utility/Blob.h"
#include "Window/IWindowEventListener.h"

class DXRenderer : public Core::IWindowEventListener
{
public:
    DXRenderer(HWND windowHandle);
    ~DXRenderer();

    virtual bool LoadContent(TaskGPU* loadTask);
    virtual void UnloadContent();

    virtual void OnUpdate(Core::Input::UpdateEvent& e) override;
    virtual void OnRender(Core::Input::RenderEvent& e, Frame& frame) override;
    virtual void OnKeyPressed(Core::Input::KeyEvent& e) override;
    virtual void OnKeyReleased(Core::Input::KeyEvent& e) override {}
    virtual void OnMouseMoved(Core::Input::MouseMoveEvent& e) override;
    virtual void OnMouseButtonPressed(Core::Input::MouseButtonEvent& e) override;
    virtual void OnMouseButtonReleased(Core::Input::MouseButtonEvent& e) override;
    virtual void OnMouseScroll(Core::Input::MouseScrollEvent& e) override;
    virtual void OnResize(Core::Input::ResizeEvent& e) override;

private:
    void transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    void clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    void clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    ComPtr<ID3D12Device2> _DXDevice;
    HWND _windowHandle;

    bool _contentLoaded;

    RootSignature _pipeline;
    RootSignature _AABBpipeline;

    Resource* _ambient;

    Scene _scene;
    Camera _camera;
    bool _isCameraMoving;
};