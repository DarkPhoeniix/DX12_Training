#pragma once

#include "Game.h"
#include "RenderWindow.h"

#include <vector>

class RenderCubeExample : public Game
{
public:
    using super = Game;

    RenderCubeExample(const std::wstring& name, int width, int height, bool vSync = false);

    virtual bool loadContent() override;
    virtual void unloadContent() override;

protected:
    virtual void onUpdate(UpdateEvent& e) override;
    virtual void onRender(RenderEvent& e) override;
    virtual void onKeyPressed(KeyEvent& e) override;
    virtual void onMouseScroll(MouseScrollEvent& e) override;
    virtual void onResize(ResizeEvent& e) override;

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

    void spawnNewCubes(size_t K);
    bool pointInExtents(const DirectX::XMVECTOR& location);
    bool pointIntersects(const DirectX::XMVECTOR& location);
    bool pointIntersectsGrid(const DirectX::XMVECTOR& location);

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
    float _spawnRadius = 10.0f;

    DirectX::XMVECTOR _minExtent = { -200.0f, -100.0f, 0.0f, 1.0f };
    DirectX::XMVECTOR _maxExtent = {  200.0f,  100.0f, 0.0f, 1.0f };

    std::vector<DirectX::XMVECTOR> _cubes;
    size_t _activeIndex;

    float _cellSize;
    float _gridWidth;
    float _gridHeight;
    size_t _cellsNumX;
    size_t _cellsNumY;
    std::vector<std::vector<int>> _grid;
};