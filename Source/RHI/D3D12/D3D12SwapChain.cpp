
#include "RHI_PCH.h"

#include "D3D12SwapChain.h"

#include "D3D12Helpers.h"

#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Texture.h"

#include "GPUCrashTracker/IGPUCrashTracker.h"

namespace rhi::d3d12
{
    D3D12SwapChain::D3D12SwapChain(Device* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
        : _dxgiSwapChain{}
        , _device(device)
        , _RTVDescriptorSize(device->GetDescriptorHandleIncrementSize(DescriptorHeapType::RTV))
        , _currentBackBufferIndex(0)
        , _windowHandle{}
        , _width(0)
        , _height(0)
        , _vSync(false)
        , _tearingSupport(CheckTearingSupport())
    {
        DescriptorHeapDescription desc =
        {
            .Type = DescriptorHeapType::RTV,
            .NumDescriptors = BACK_BUFFER_COUNT,
            .ShaderVisible = false,
            .Flags = 0
        };

        _RTVDescriptorHeap = device->CreateDescriptorHeap(desc);

        _windowHandle = windowHandle;
        _width = width;
        _height = height;
        _vSync = vSync;

        _dxgiSwapChain = CreateSwapChain();
        _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

        UpdateRenderTargetViews();
    }

    D3D12SwapChain::D3D12SwapChain(D3D12SwapChain&& other) noexcept
        : _dxgiSwapChain(std::move(other._dxgiSwapChain))
        , _device(std::move(other._device))
        , _RTVDescriptorSize(other._RTVDescriptorSize)
        , _currentBackBufferIndex(other._currentBackBufferIndex)
        , _windowHandle(std::move(other._windowHandle))
        , _width(other._width)
        , _height(other._height)
        , _vSync(other._vSync)
        , _tearingSupport(other._tearingSupport)
    {
    }

    D3D12SwapChain::~D3D12SwapChain()
    {
    }

    D3D12SwapChain& D3D12SwapChain::operator=(D3D12SwapChain&& other) noexcept
    {
        if (this != &other)
        {
            _dxgiSwapChain = std::move(other._dxgiSwapChain);
            _device = std::move(other._device);
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

    std::shared_ptr<Texture> D3D12SwapChain::GetBuffer(std::uint32_t index)
    {
        return _backBuffers[index];
    }

    std::shared_ptr<Texture> D3D12SwapChain::GetBackBuffer()
    {
        return _backBuffers[_currentBackBufferIndex];
    }

    std::uint32_t D3D12SwapChain::Present()
    {
        UINT syncInterval = _vSync ? 1 : 0;
        UINT presentFlags = (_tearingSupport && !_vSync) ? DXGI_PRESENT_ALLOW_TEARING : 0;
        HRESULT result = _dxgiSwapChain->Present(syncInterval, presentFlags);

        if (FAILED(result))
        {
            LOG_CRITICAL("Failed to present swap chain. HRESULT: 0x{:X}", result);
            // Crash tracker need some time to process the crash dump.
            _device->GetCrashTracker()->WaitUntilCrashDumpFinished();
            // Terminate on failure
            exit(-1);
        }

        _currentBackBufferIndex = static_cast<std::uint32_t>(_dxgiSwapChain->GetCurrentBackBufferIndex());

        return _currentBackBufferIndex;
    }

    void D3D12SwapChain::OnResize(std::uint32_t width, std::uint32_t height)
    {
        if (_width != width || _height != height)
        {
            _width = std::max((std::uint32_t)1, width);
            _height = std::max((std::uint32_t)1, height);

            for (int i = 0; i < BACK_BUFFER_COUNT; ++i)
            {
                _backBuffers[i].reset();
            }

            DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
            HRESULT getDescResult = _dxgiSwapChain->GetDesc(&swapChainDesc);
            CHECK(getDescResult, "Failed to get swap chain description.");

            HRESULT resizeResult = _dxgiSwapChain->ResizeBuffers(BACK_BUFFER_COUNT, _width, _height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
            CHECK(resizeResult, "Failed to resize swap chain buffers.");

            _currentBackBufferIndex = _dxgiSwapChain->GetCurrentBackBufferIndex();

            UpdateRenderTargetViews();
        }
    }

    ScissorRect D3D12SwapChain::GetDesktopCoordinates()
    {
        ComPtr<IDXGIOutput> containingOutput;
        _dxgiSwapChain->GetContainingOutput(&containingOutput);

        DXGI_OUTPUT_DESC outputDescription;
        HRESULT result = containingOutput->GetDesc(&outputDescription);
        CHECK(result, "Failed to get swap chain description.");

        ScissorRect rect =
        {
            .Left = outputDescription.DesktopCoordinates.left,
            .Top = outputDescription.DesktopCoordinates.top,
            .Right = outputDescription.DesktopCoordinates.right,
            .Bottom = outputDescription.DesktopCoordinates.bottom,
        };

        return rect;
    }

    void* D3D12SwapChain::GetNative() const
    {
        return static_cast<void*>(_dxgiSwapChain.Get());
    }

    ComPtr<IDXGISwapChain4> D3D12SwapChain::CreateSwapChain()
    {
        ComPtr<IDXGISwapChain4> dxgiSwapChain4;
        ComPtr<IDXGIFactory4> dxgiFactory4;
        UINT createFactoryFlags = 0;
#if defined(ENABLE_DEVICE_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif // ENABLE_DEVICE_DEBUG

        HRESULT createDXGIFactoryResult = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4));
        CHECK(createDXGIFactoryResult, "Failed to create DXGI factory.");

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

        ID3D12CommandQueue* queue = D3D12Cast<ID3D12CommandQueue>(_device->GetGraphicsQueue()->GetNative());

        ComPtr<IDXGISwapChain1> swapChain1;
        HRESULT createSwapChainResult = dxgiFactory4->CreateSwapChainForHwnd(
            queue,
            _windowHandle,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1);
        CHECK(createSwapChainResult, "Failed to create swap chain for window.");

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        HRESULT makeAssociationResult = dxgiFactory4->MakeWindowAssociation(_windowHandle, DXGI_MWA_NO_ALT_ENTER);
        CHECK(makeAssociationResult, "Failed to make window association for swap chain.");

        HRESULT result = swapChain1.As(&dxgiSwapChain4);
        CHECK(result, "Failed to cast IDXGISwapChain1 to IDXGISwapChain4.");

        _currentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

        return dxgiSwapChain4;
    }

    bool D3D12SwapChain::CheckTearingSupport() const
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
            {
                factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            }
        }

        return allowTearing == TRUE;
    }

    void D3D12SwapChain::UpdateRenderTargetViews()
    {
        CPUDescriptor heapStart = _RTVDescriptorHeap->GetHeapStartCPUHandle();

        for (int i = 0; i < BACK_BUFFER_COUNT; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            HRESULT result = _dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
            CHECK(result, "Failed to get back buffer from swap chain.");

            _backBuffers[i] = _device->CreateTexture(backBuffer.Get());
            _device->CreateTextureRTV(_backBuffers[i], heapStart);

            heapStart.Offset(_RTVDescriptorSize);
        }
    }
} // namespace rhi::d3d12
