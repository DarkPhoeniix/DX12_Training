#pragma once

#include "DescriptorHeap.h"

namespace core
{
    class Win32Window;
}

namespace dx12
{
    constexpr UINT BACK_BUFFER_COUNT = 3;

    class SwapChain
    {
    public:
        SwapChain();
        ~SwapChain();

        void Init(const core::Win32Window& window);

        DXGI_SWAP_CHAIN_DESC GetDescription() const;

        Resource* GetBuffer(unsigned int index);
        Resource* GetBackBuffer();

        void UpdateRenderTargetViews();
        UINT Present();

        void OnResize(const DirectX::XMUINT2& size);

        ComPtr<IDXGIOutput> GetContainingOutput();

    private:
        ComPtr<IDXGISwapChain4> CreateSwapChain();
        bool CheckTearingSupport() const;

        DXGI_SWAP_CHAIN_DESC _swapChainDesc;
        ComPtr<IDXGISwapChain4> _dxgiSwapChain;
        DescriptorHeap _RTVDescriptorHeap;
        UINT _RTVDescriptorSize;

        Resource _backBuffers[BACK_BUFFER_COUNT];
        UINT _currentBackBufferIndex;

        HWND _windowHandle;
        int _width;
        int _height;

        bool _vSync;
        bool _tearingSupport;
    };
} // namespace dx12
