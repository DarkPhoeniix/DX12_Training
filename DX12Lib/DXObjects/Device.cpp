#include "stdafx.h"

#include "Device.h"

namespace Core
{
    namespace
    {
        void EnableDXDebugLayer()
        {
            ComPtr<ID3D12Debug> debugInterface;
            Helper::throwIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
            debugInterface->EnableDebugLayer();

            ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
            D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings));

            // Turn on auto-breadcrumbs and page fault reporting.
            pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

            Logger::Log(LogType::Info, "Enabled DX Debug Layer");
        }
    } // namespace

    Device* Device::_instance = nullptr;

    void Device::Init()
    {
        if (_instance)
        {
            Logger::Log(LogType::Warning, "Device has already been initialized");
        }
        else
        {
            _instance = new Device();
        }
    }

    void Device::Destroy()
    {
        if (_instance)
        {
            delete _instance;
        }

        _instance = nullptr;
    }

    Device* Device::Instance()
    {
        return _instance;
    }

    ComPtr<ID3D12Device2> Device::GetDXDevice()
    {
        return _instance->_device;
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

    Device::Device()
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

        Helper::throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ComPtr<IDXGIAdapter4> dxgiAdapter4;

        if (useWarp)
        {
            Helper::throwIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
            Helper::throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
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
                    Helper::throwIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
                }
            }
        }

        _adapter = dxgiAdapter4;
    }

    void Device::CreateDevice()
    {
        Helper::throwIfFailed(D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)));
        _device->SetName(L"DX12 Device");

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

            Helper::throwIfFailed(infoQueue->PushStorageFilter(&NewFilter));
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

        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queueStream));

        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        _device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queueCopy));
    }
} // namespace Core
