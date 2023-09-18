#pragma once

#include "IRenderWindow.h"

const uint8_t g_numFrames = 3;          // The number of swap chain back buffers

class RenderWindow : public IRenderWindow
{
public:
    RenderWindow(std::uint32_t width, std::uint32_t height, const std::wstring& name);

    void onInit() override;
    void onUpdate() override;
    void onRender() override;
    void onDestroy() override;

private:
    void loadPipeline();
    void populateCommandList();
    void waitForPreviousFrame();
    void updateRenderTargetViews();

private:
    // DirectX 12 Objects
    ComPtr<ID3D12Device2> _device;
    ComPtr<ID3D12CommandQueue> _commandQueue;
    ComPtr<IDXGISwapChain4> _swapChain;
    ComPtr<ID3D12Resource> _backBuffers[g_numFrames];
    ComPtr<ID3D12GraphicsCommandList> _commandList;
    ComPtr<ID3D12CommandAllocator> _commandAllocators[g_numFrames];
    ComPtr<ID3D12DescriptorHeap> _RTVDescriptorHeap;
    UINT _RTVDescriptorSize;
    UINT _currentBackBufferIndex;

    // Synchronization objects
    ComPtr<ID3D12Fence> _fence;
    std::uint64_t _fenceValue = 0;
    std::uint64_t _frameFenceValues[g_numFrames] = {};
    HANDLE _fenceEvent = nullptr;
};

