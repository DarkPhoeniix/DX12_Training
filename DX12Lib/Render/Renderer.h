#pragma once

#include "Interfaces/IGame.h"
#include "RenderWindow.h"
#include "Objects/Camera.h"
#include "Utility/PipelineSettings.h"
#include "FencePool.h"
#include "Objects/Scene.h"
#include "Frame.h"
#include "Blob.h"

#include "DescriptorHeap.h"
#include "Heap.h"

class Renderer : public IGame
{
public:
    using super = IGame;

    Renderer(const std::wstring& name, int width, int height, bool vSync = false);
    ~Renderer();

    virtual bool LoadContent() override;
    virtual void UnloadContent() override;

protected:
    void onUpdate(UpdateEvent& e) override;
    void onRender(RenderEvent& e) override;
    void onKeyPressed(KeyEvent& e) override;
    void onMouseScroll(MouseScrollEvent& e) override;
    void onMouseMoved(MouseMoveEvent& e) override;
    void onMouseButtonPressed(MouseButtonEvent& e) override;
    void onMouseButtonReleased(MouseButtonEvent& e) override;
    void onResize(ResizeEvent& e) override;

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
};