#pragma once

#include "Window/IWindowEventListener.h"
#include "Window/Win32Window.h"
#include "Events/ResizeEvent.h"

class SwapChain : public Core::IWindowEventListener
{
public:
    SwapChain(const Core::Win32Window& window);
    ~SwapChain();

    void UpdateRenderTargetViews();
    UINT Present();

    void OnResize(ResizeEvent& e) override;

private:
    ComPtr<IDXGISwapChain4> CreateSwapChain();
    bool CheckTearingSupport() const;

    ComPtr<IDXGISwapChain4> _dxgiSwapChain;
    UINT _currentBackBufferIndex;

    HWND _windowHandle;
    int _width;
    int _height;

    bool _vSync;
    bool _tearingSupported;
};
