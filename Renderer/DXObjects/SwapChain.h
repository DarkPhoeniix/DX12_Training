#pragma once

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/Resource.h"
#include "Window/IWindowEventListener.h"
#include "Window/Win32Window.h"

class Core::Events::ResizeEvent;
namespace Core
{
    constexpr UINT BACK_BUFFER_COUNT = 3;

    class SwapChain : public Core::Events::IWindowEventListener
    {
    public:
        SwapChain();
        ~SwapChain();

        void Init(std::shared_ptr<Core::Win32Window> window);

        DXGI_SWAP_CHAIN_DESC GetDescription() const;

        void GetBuffer(unsigned int index, Resource& resource) const;

        void UpdateRenderTargetViews();
        UINT Present();

        void OnResize(Core::Events::ResizeEvent& e) override;

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
} // namespace Core
