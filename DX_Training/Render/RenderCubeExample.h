#pragma once

#include "Interfaces/IGame.h"
#include "RenderWindow.h"

#include "PoissonDiskDistribution.h"
#include "Camera.h"

#include <vector>

class RenderCubeExample : public IGame
{
public:
    using super = IGame;

    RenderCubeExample(const std::wstring& name, int width, int height, bool vSync = false);

    virtual bool loadContent() override;
    virtual void unloadContent() override;

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
    void transitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    void clearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    void clearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    void updateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void resizeDepthBuffer(int width, int height);

    uint64_t _fenceValues[Window::BUFFER_COUNT] = {};

    Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _DSVHeap;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    float _FoV;

    DirectX::XMMATRIX _modelMatrix;
    DirectX::XMMATRIX _viewMatrix;
    DirectX::XMMATRIX _projectionMatrix;

    bool _contentLoaded;

    float _spawnRate = 50.0f;
    float _deltaTimeLastSpawn = 0.0f;

    PoissonDiskDistribution distribution;

    bool _isCameraMoving = false;
    Camera camera;
};