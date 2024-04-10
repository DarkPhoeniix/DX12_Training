#include "stdafx.h"

#include "SwapChain.h"

namespace
{
    constexpr UINT BUFFER_COUNT = 3;
}

SwapChain::SwapChain(const Core::Win32Window& window)
    : _dxgiSwapChain(CreateSwapChain())
    , _currentBackBufferIndex(0)
    , _windowHandle(window.GetWindowHandle())
    , _width(window.GetWidth())
    , _height(window.GetHeight())
    , _vSync(window.IsVSync())
    , _tearingSupported(CheckTearingSupport())
{   }

SwapChain::~SwapChain()
{
    _dxgiSwapChain = nullptr;
}

void SwapChain::UpdateRenderTargetViews()
{
    // TODO: Not implemented
}

UINT SwapChain::Present()
{
    // TODO: Not implemented
    return 0;
}

void SwapChain::OnResize(ResizeEvent& e)
{
}

ComPtr<IDXGISwapChain4> SwapChain::CreateSwapChain()
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    Helper::throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = _width;
    swapChainDesc.Height = _height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BUFFER_COUNT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = _tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    Helper::throwIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        pShared->queueStream.Get(), // TODO: how to pass stream queue to swap chain?
        _windowHandle,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    Helper::throwIfFailed(dxgiFactory4->MakeWindowAssociation(_windowHandle, DXGI_MWA_NO_ALT_ENTER));

    Helper::throwIfFailed(swapChain1.As(&dxgiSwapChain4));

    _currentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

    return dxgiSwapChain4;
}

bool SwapChain::CheckTearingSupport() const
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
            factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    }

    return allowTearing == TRUE;
}
