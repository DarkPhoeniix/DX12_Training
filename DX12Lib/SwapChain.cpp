#include "DX12LibPCH.h"

#include "SwapChain.h"

#include "IGPUCrashTracker.h"

#include "Window/Win32Window.h"

namespace dx12
{
    SwapChain::SwapChain()
        : _dxgiSwapChain{}
        , _swapChainDesc()
        , _RTVDescriptorSize(dx12::Device::GetDXDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
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

        _RTVDescriptorHeap.SetDescription(desc);
        _RTVDescriptorHeap.Create();
    }

    SwapChain::SwapChain(SwapChain&& other) noexcept
        : _dxgiSwapChain(std::move(other._dxgiSwapChain))
        , _swapChainDesc(other._swapChainDesc)
        , _RTVDescriptorSize(other._RTVDescriptorSize)
        , _currentBackBufferIndex(other._currentBackBufferIndex)
        , _windowHandle(std::move(other._windowHandle))
        , _width(other._width)
        , _height(other._height)
        , _vSync(other._vSync)
        , _tearingSupport(other._tearingSupport)
    {
    }

    SwapChain::~SwapChain()
    {
        _dxgiSwapChain = nullptr;
    }

    SwapChain& SwapChain::operator=(SwapChain&& other) noexcept
    {
        if (this != &other)
        {
            _dxgiSwapChain = std::move(other._dxgiSwapChain);
            _swapChainDesc = other._swapChainDesc;
            _RTVDescriptorSize = other._RTVDescriptorSize;
            _currentBackBufferIndex = other._currentBackBufferIndex;
            _windowHandle = std::move(other._windowHandle);
            _width = other._width;
            _height = other._height;
            _vSync = other._vSync;
            _tearingSupport = other._tearingSupport;
        }

        return *this;
    }

    void SwapChain::Init(const core::Win32Window& window)
    {
        _windowHandle = window.GetWindowHandle();
        _width = window.GetWidth();
        _height = window.GetHeight();
        _vSync = window.IsVSync();

        _dxgiSwapChain = CreateSwapChain();

        helpers::throwIfFailed(_dxgiSwapChain->GetDesc(&_swapChainDesc));
        helpers::throwIfFailed(_dxgiSwapChain->ResizeBuffers(BACK_BUFFER_COUNT, _width, _height, _swapChainDesc.BufferDesc.Format, _swapChainDesc.Flags));

        _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews();
    }

    DXGI_SWAP_CHAIN_DESC SwapChain::GetDescription() const
    {
        return _swapChainDesc;
    }

    Resource* SwapChain::GetBuffer(std::uint32_t index)
    {
        return &_backBuffers[index];
    }

    Resource* SwapChain::GetBackBuffer()
    {
        return &_backBuffers[_currentBackBufferIndex];
    }

    void SwapChain::UpdateRenderTargetViews()
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_RTVDescriptorHeap.GetHeapStartCPUHandle());

        for (int i = 0; i < BACK_BUFFER_COUNT; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            helpers::throwIfFailed(_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
            dx12::Device::GetDXDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

            _backBuffers[i].InitFromDXResource(backBuffer);

            rtvHandle.Offset(_RTVDescriptorSize);
        }
    }

    UINT SwapChain::Present()
    {
        UINT syncInterval = _vSync ? 1 : 0;
        UINT presentFlags = (_tearingSupport && !_vSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
        HRESULT result = _dxgiSwapChain->Present(syncInterval, presentFlags);

        if (FAILED(result))
        {
            // Crash tracker need some time to process the crash dump.
            Device::GetCrashTracker()->WaitUntilCrashDumpFinished();
            // Terminate on failure
            exit(-1);
        }

        _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

        return _currentBackBufferIndex;
    }

    void SwapChain::OnResize(const DirectX::XMUINT2& size)
    {
        if (_width != size.x || _height != size.y)
        {
            _width = std::max(1, (int)size.x);
            _height = std::max(1, (int)size.y);

            for (int i = 0; i < BACK_BUFFER_COUNT; ++i)
            {
                _backBuffers[i].GetDXResource().Reset();
            }

            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            helpers::throwIfFailed(_dxgiSwapChain->GetDesc(&swapChainDesc));
            helpers::throwIfFailed(_dxgiSwapChain->ResizeBuffers(BACK_BUFFER_COUNT, _width,
                _height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

            _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

            UpdateRenderTargetViews();
        }
    }

    ComPtr<IDXGISwapChain4> SwapChain::CreateSwapChain()
    {
        ComPtr<IDXGISwapChain4> dxgiSwapChain4;
        ComPtr<IDXGIFactory4> dxgiFactory4;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        helpers::throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

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

        ID3D12CommandQueue* queue = dx12::Device::GetStreamQueue();

        ComPtr<IDXGISwapChain1> swapChain1;
        helpers::throwIfFailed(dxgiFactory4->CreateSwapChainForHwnd(
            queue,
            _windowHandle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1));

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        helpers::throwIfFailed(dxgiFactory4->MakeWindowAssociation(_windowHandle, DXGI_MWA_NO_ALT_ENTER));
        helpers::throwIfFailed(swapChain1.As(&dxgiSwapChain4));

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

    ComPtr<IDXGIOutput> SwapChain::GetContainingOutput()
    {
        ComPtr<IDXGIOutput> output;
        helpers::throwIfFailed(_dxgiSwapChain->GetContainingOutput(&output));
        return output;
    }
} // namespace dx12
