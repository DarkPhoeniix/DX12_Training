#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "DXObjects/Texture.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "Render/Frame.h"
#include "Render/FencePool.h"
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

private:
    void transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    ComPtr<ID3D12Device2> _DXDevice;
    HWND _windowHandle;

    bool _contentLoaded;

    RootSignature _pipeline;
    RootSignature _AABBpipeline;

    std::shared_ptr<Resource> _ambient;

    DescriptorHeap _texDescHeap;
    std::shared_ptr<Texture> _tex;

    Scene _scene;
    Camera _camera;
    bool _isCameraMoving;
};