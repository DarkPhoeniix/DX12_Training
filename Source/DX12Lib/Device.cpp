#include "DX12LibPCH.h"

#include "Device.h"

#include "IGPUCrashTracker.h"

namespace dx12
{
    namespace
    {
        void EnableDXDebugLayer()
        {
            ComPtr<ID3D12Debug> debugInterface;
            HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
            CHECK(result, "Failed to get D3D12 debug interface.");
            debugInterface->EnableDebugLayer();

            ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
            D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings));

            // Turn on auto-breadcrumbs and page fault reporting.
            pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

            LOG_INFO("DirectX 12 debug layer enabled.");
        }
    } // namespace

    Device* Device::_instance = nullptr;

    Device::Device(Device&& other) noexcept
        : _device(std::move(other._device))
        , _adapter(std::move(other._adapter))
        , _queueStream(std::move(other._queueStream))
        , _queueCompute(std::move(other._queueCompute))
        , _queueCopy(std::move(other._queueCopy))
        , _swapChain(std::move(other._swapChain))
    {
    }

    Device& Device::operator=(Device&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _adapter = std::move(other._adapter);
            _queueStream = std::move(other._queueStream);
            _queueCompute = std::move(other._queueCompute);
            _queueCopy = std::move(other._queueCopy);
            _swapChain = std::move(other._swapChain);
        }

        return *this;
    }

    void Device::Init()
    {
        if (_instance)
        {
            LOG_WARNING("Trying to reinitialize DX12 device, skipping.");
        }
        else
        {
            _instance = new Device();
            LOG_INFO("DX12 device initialized.");
        }
    }

    void Device::Destroy()
    {
        if (_instance)
        {
            delete _instance;
        }

        _instance = nullptr;

        LOG_INFO("DX12 device destroyed.");
    }

    void Device::BindSwapChain(SwapChain* swapChain)
    {
        ASSERT(swapChain, "SwapChain is nullptr when trying to bind it to the device.");
        _instance->_swapChain = swapChain;
    }

    ComPtr<ID3D12Device2> Device::GetDXDevice()
    {
        return _instance->_device;
    }

    ComPtr<IDXGIAdapter4> Device::GetDXAdapter()
    {
        return _instance->_adapter;
    }

    ID3D12CommandQueue* Device::GetStreamQueue()
    {
        return _instance->_queueStream.Get();
    }

    ID3D12CommandQueue* Device::GetComputeQueue()
    {
        return _instance->_queueCompute.Get();
    }

    ID3D12CommandQueue* Device::GetCopyQueue()
    {
        return _instance->_queueCopy.Get();
    }

    void Device::OnResize(const DirectX::XMUINT2& size)
    {
        _instance->_swapChain->OnResize(size);
    }

    dx12::Resource* Device::GetBackBuffer()
    {
        return _instance->_swapChain->GetBackBuffer();
    }

    void Device::Present()
    {
        _instance->_swapChain->Present();
    }

    void Device::CreateRenderTargetView(const RenderTargetView& view, DescriptorHeap& descriptorHeap)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
        _instance->_device->CreateRenderTargetView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    }

    void Device::CreateDepthStencilView(const DepthStencilView& view, DescriptorHeap& descriptorHeap)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
        _instance->_device->CreateDepthStencilView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    }

    void Device::CreateConstantBufferView(const ConstantBufferView& view, DescriptorHeap& descriptorHeap)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
        _instance->_device->CreateConstantBufferView(&view, heapHandle);
    }

    void Device::CreateShaderResourceView(const ShaderResourceView& view, DescriptorHeap& descriptorHeap)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
        _instance->_device->CreateShaderResourceView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    }

    void Device::CreateUnorderedAccessView(const UnorderedAccessView& view, DescriptorHeap& descriptorHeap, dx12::Resource* counterResource)
    {
        ID3D12Resource* counter = counterResource ? counterResource->GetDXResource().Get() : nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
        _instance->_device->CreateUnorderedAccessView(view.Owner->GetDXResource().Get(), counter, &view, heapHandle);
    }

    std::shared_ptr<tracking::IGPUCrashTracker> Device::GetCrashTracker()
    {
        return _instance->_crashTracker;
    }

    Device::Device()
        : _crashTracker(tracking::IGPUCrashTracker::Create())
    {
#if defined(_DEBUG)
        EnableDXDebugLayer();
#endif

        CreateAdapter();
        CreateDevice();
        CreateQueues();
    }

    Device::~Device()
    {
        _adapter = nullptr;
        _device = nullptr;

        _queueCompute = nullptr;
        _queueStream = nullptr;
        _queueCopy = nullptr;
    }

    void Device::CreateAdapter(bool useWarp)
    {
        ComPtr<IDXGIFactory4> dxgiFactory;
        UINT createFactoryFlags = 0;
#if defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        HRESULT createDXGIFactoryResult = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
        CHECK(createDXGIFactoryResult, "Failed to create DXGI factory.");

        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ComPtr<IDXGIAdapter4> dxgiAdapter4;

        if (useWarp)
        {
            HRESULT enumWardAdapterResult = dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1));
            CHECK(enumWardAdapterResult, "Failed to enumerate WARP adapter.");

            HRESULT result = dxgiAdapter1.As(&dxgiAdapter4);
            CHECK(result, "Failed to cast WARP adapter to IDXGIAdapter4.");
        }
        else
        {
            SIZE_T maxDedicatedVideoMemory = 0;
            for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
                dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

                // Check to see if the adapter can create a D3D12 device without actually 
                // creating it. The adapter with the largest dedicated video memory
                // is favored.
                if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                    SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                        D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) &&
                    dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                    HRESULT result = dxgiAdapter1.As(&dxgiAdapter4);
                    CHECK(result, "Failed to cast adapter to IDXGIAdapter4.");
                }
            }
        }

        _adapter = dxgiAdapter4;

        LOG_INFO("DX12 Adapter created.");
    }

    void Device::CreateDevice()
    {
        _crashTracker->Enable();

        HRESULT createDeviceResult = D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device));
        CHECK(createDeviceResult, "Failed to create D3D12 device.");
        _device->SetName(L"DX12 Device");

        _crashTracker->Initialize(_device.Get());

        // Enable debug messages in debug mode.
#if defined(_DEBUG)
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(_device.As(&infoQueue)))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            // Suppress whole categories of messages
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            HRESULT result = infoQueue->PushStorageFilter(&NewFilter);
            CHECK(result, "Failed to set D3D12 info queue filter.");
        }
#endif
    }

    void Device::CreateQueues()
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        desc.Priority = 0;

        desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queueCompute));
        _queueCompute->SetName(L"Compute Queue");

        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queueStream));
        _queueStream->SetName(L"Stream Queue");

        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queueCopy));
        _queueCopy->SetName(L"Copy Queue");

        LOG_INFO("DX12 Command Queues created.");
    }
} // namespace dx12
