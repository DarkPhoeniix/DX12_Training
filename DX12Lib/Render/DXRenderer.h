#pragma once

#include "RenderWindow.h"
#include "Scene/Camera.h"
#include "FencePool.h"
#include "Scene/Scene.h"
#include "Frame.h"
#include "Blob.h"
#include "Utility/PipelineSettings.h"
#include "Window/IWindowEventListener.h"

#include "DescriptorHeap.h"
#include "Heap.h"

class DXRenderer : public Core::IWindowEventListener
{
public:
    DXRenderer(HWND windowHandle);
    ~DXRenderer();

    virtual bool LoadContent();
    virtual void UnloadContent();

    virtual void OnUpdate(UpdateEvent& e) override;
    virtual void OnRender(RenderEvent& e) override;
    virtual void OnKeyPressed(KeyEvent& e) override;
    virtual void OnKeyReleased(KeyEvent& e) override;
    virtual void OnMouseMoved(MouseMoveEvent& e) override;
    virtual void OnMouseButtonPressed(MouseButtonEvent& e) override;
    virtual void OnMouseButtonReleased(MouseButtonEvent& e) override;
    virtual void OnMouseScroll(MouseScrollEvent& e) override;

    virtual void onResize(ResizeEvent& e);

private:
    void transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    void clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    void clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    void updateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void resizeDepthBuffer(int width, int height);

    uint64_t _fenceValues[Window::BUFFER_COUNT] = {};

    ComPtr<ID3D12Resource> _depthBuffer;
    ComPtr<ID3D12DescriptorHeap> _DSVHeap;

    PipelineSettings _pipeline;
    PipelineSettings _AABBpipeline;

    ComPtr<ID3D12RootSignature> _rootComputeSignature;
    ComPtr<ID3D12PipelineState> _pipelineComputeState;

    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    bool _contentLoaded;

    Resource* _ambient;
    Resource* _cubeTransformsRes[3];
    DirectX::XMMATRIX* _transfP[3];

    Resource* _dynamicData;
    Resource* _UAVRes;
    ComPtr<ID3D12Heap> _pHeap;
    ComPtr<ID3D12DescriptorHeap> _descHeap;

    bool _isCameraMoving = false;
    Camera _camera;

    Frame _frames[3];
    Frame* _current;
    AllocatorPool _allocs;
    FencePool _fencePool;

    Resource* _tex;
    Base::Blob _blob;
    ComPtr<ID3D12Resource> _intermediateTex;
    Scene _scene;

    std::shared_ptr<Heap> _texturesHeap;
    std::shared_ptr<DescriptorHeap> _texturesDescHeap;





    ID3D12Device2* _DXDevice;
};