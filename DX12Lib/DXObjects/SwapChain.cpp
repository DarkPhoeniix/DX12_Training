#include "stdafx.h"

#include "SwapChain.h"

#include "Device.h"

SwapChain::SwapChain()
    : _DXDevice(Core::Device::GetDXDevice())
    , _dxgiSwapChain{}
    , _RTVDescriptorSize(_DXDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
    , _currentBackBufferIndex(0)
    , _windowHandle{}
    , _width(0)
    , _height(0)
    , _vSync(false)
    , _tearingSupport(CheckTearingSupport())
{
    DescriptorHeapDescription desc;
    desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    desc.SetNumDescriptors(BACK_BUFFER_COUNT);
    desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    desc.SetNodeMask(0);

    _RTVDescriptorHeap.SetDevice(_DXDevice);
    _RTVDescriptorHeap.SetDescription(desc);
    _RTVDescriptorHeap.Create();
}

SwapChain::~SwapChain()
{
    _dxgiSwapChain = nullptr;
}

void SwapChain::Init(std::shared_ptr<Core::Win32Window> window)
{
    _windowHandle = window->GetWindowHandle();
    _width = window->GetWidth();
    _height = window->GetHeight();
    _vSync = window->IsVSync();

    _dxgiSwapChain = CreateSwapChain();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    Helper::throwIfFailed(_dxgiSwapChain->GetDesc(&swapChainDesc));
    Helper::throwIfFailed(_dxgiSwapChain->ResizeBuffers(BACK_BUFFER_COUNT, _width,
        _height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

    _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

    UpdateRenderTargetViews();
}

void SwapChain::GetBuffer(unsigned int index, Resource& resource) const
{
    ComPtr<ID3D12Resource> buffer;
    _dxgiSwapChain->GetBuffer(index, IID_PPV_ARGS(&buffer));
    resource.InitFromDXResource(buffer);
}

void SwapChain::UpdateRenderTargetViews()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_RTVDescriptorHeap.GetHeapStartCPUHandle());

    for (int i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        Helper::throwIfFailed(_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
        _DXDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        _backBuffers[i].InitFromDXResource(backBuffer);

        rtvHandle.Offset(_RTVDescriptorSize);
    }
}

UINT SwapChain::Present()
{
    UINT syncInterval = _vSync ? 1 : 0;
    UINT presentFlags = (_tearingSupport && !_vSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
    Helper::throwIfFailed(_dxgiSwapChain->Present(syncInterval, presentFlags));
    _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

    return _currentBackBufferIndex;
}

void SwapChain::OnResize(Core::Input::ResizeEvent& e)
{
    if (_width != e.width || _height != e.height)
    {
        _width = std::max(1, e.width);
        _height = std::max(1, e.height);
        
        // TODO: Do it really needs Reset() ?
        //for (int i = 0; i < BUFFER_COUNT; ++i)
        //{
        //    _backBuffers[i].Reset(); 
        //}

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        Helper::throwIfFailed(_dxgiSwapChain->GetDesc(&swapChainDesc));
        Helper::throwIfFailed(_dxgiSwapChain->ResizeBuffers(BACK_BUFFER_COUNT, _width,
            _height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews();
    }
}

void SwapChain::SetDXDevice(ComPtr<ID3D12Device2> device)
{
    _DXDevice = device;
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
    swapChainDesc.BufferCount = BACK_BUFFER_COUNT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = _tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ID3D12CommandQueue* queue = Core::Device::GetStreamQueue();

    ComPtr<IDXGISwapChain1> swapChain1;
    Helper::throwIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
        queue,
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
