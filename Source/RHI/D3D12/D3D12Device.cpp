
#include "RHI_PCH.h"

#include "D3D12Device.h"

#include "D3D12Buffer.h"
#include "D3D12CommandList.h"
#include "D3D12CommandListPool.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandSignature.h"
#include "D3D12DescriptorHeap.h"
#include "D3D12Descriptor.h"
#include "D3D12Helpers.h"
#include "D3D12Fence.h"
#include "D3D12QueryHeap.h"
#include "D3D12PipelineState.h"
#include "D3D12SwapChain.h"
#include "D3D12Texture.h"
#include "IGPUCrashTracker.h"
#include "D3D12StatisticsQuery.h"
#include "D3D12TimestampQuery.h"
#include "D3D12PipelineState.h"

#include "BufferView.h"
#include "TextureView.h"
#include "ResourceCommon.h"
#include "DescriptorHeap.h"
#include "SwapChain.h"

#include <D3D12MemAlloc.h>

#include <dxgidebug.h>

namespace rhi::d3d12
{
    namespace
    {
        std::string ConvertWCharToString(const WCHAR* wideStr)
        {
            if (!wideStr) return "";

            size_t size_needed = 128;
            std::string result(size_needed, 0);
            size_t i;
            wcstombs_s(&i, &result[0], size_needed, wideStr, size_needed - 1);

            return result;
        }

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

        void ReportLiveObjects()
        {
            IDXGIDebug* dxgiDebug;
            HRESULT result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
            CHECK(result, "Failed to get DXGI debug interface.");

            if (SUCCEEDED(result))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
            }

            dxgiDebug->Release();
        }
    } // namespace unnamed

    D3D12Device::D3D12Device()
        : _crashTracker(tracking::IGPUCrashTracker::Create())
    {
#if ENABLE_DEVICE_DEBUG
        EnableDXDebugLayer();
#endif // ENABLE_DEVICE_DEBUG

        CreateAdapter();
        CreateDevice();
        CreateAllocator();
        CreateQueues();

        CheckFeatureSupport();

#if ENABLE_DEVICE_DEBUG
        std::atexit(ReportLiveObjects);
#endif // ENABLE_DEVICE_DEBUG

        LOG_INFO("DX12 device initialized.");
    }

    D3D12Device::~D3D12Device()
    {
        // The allocator holds references to the device and adapter, so it must be released first
        _allocator = nullptr;
        _adapter = nullptr;
        _device = nullptr;

        LOG_INFO("DX12 device destroyed.");
    }

    D3D12Device::D3D12Device(D3D12Device&& other) noexcept
        : _device(std::move(other._device))
        , _adapter(std::move(other._adapter))
        , _allocator(std::move(other._allocator))
        , _queueGraphics(std::move(other._queueGraphics))
        , _queueCompute(std::move(other._queueCompute))
        , _queueCopy(std::move(other._queueCopy))
        , _swapChain(std::move(other._swapChain))
        , _enhancedBarriersSupported(other._enhancedBarriersSupported)
    {
    }

    D3D12Device& D3D12Device::operator=(D3D12Device&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _adapter = std::move(other._adapter);
            _allocator = std::move(other._allocator);
            _queueGraphics = std::move(other._queueGraphics);
            _queueCompute = std::move(other._queueCompute);
            _queueCopy = std::move(other._queueCopy);
            _swapChain = std::move(other._swapChain);
            _enhancedBarriersSupported = other._enhancedBarriersSupported;
        }

        return *this;
    }

    BackendAPI D3D12Device::GetBackend() const
    {
        return BackendAPI::D3D12;
    }

    bool D3D12Device::IsEnhancedBarriersSupported()
    {
        return _enhancedBarriersSupported;
    }

    void D3D12Device::BindSwapChain(rhi::SwapChain* swapChain)
    {
        _swapChain = swapChain;
    }

    CommandQueue* D3D12Device::GetQueue(rhi::CommandListType type)
    {
        switch (type)
        {
        case rhi::CommandListType::Graphics:
            return GetGraphicsQueue();
        case rhi::CommandListType::Compute:
            return GetComputeQueue();
        case rhi::CommandListType::Copy:
            return GetCopyQueue();
        default:
            UNREACHABLE("Unsupported command queue type.");
            return GetGraphicsQueue();
        }
    }

    rhi::CommandQueue* D3D12Device::GetGraphicsQueue()
    {
        return _queueGraphics.get();
    }

    rhi::CommandQueue* D3D12Device::GetComputeQueue()
    {
        return _queueCompute.get();
    }

    rhi::CommandQueue* D3D12Device::GetCopyQueue()
    {
        return _queueCopy.get();
    }

    void D3D12Device::OnResize(std::uint32_t width, std::uint32_t height)
    {
        _swapChain->OnResize(width, height);
    }

    std::shared_ptr<rhi::Texture> D3D12Device::GetBackBuffer()
    {
        return _swapChain->GetBackBuffer();
    }

    void D3D12Device::Present()
    {
        _swapChain->Present();
    }

    std::shared_ptr<rhi::Buffer> D3D12Device::CreateBuffer(const rhi::BufferDescription& description, ResourceState initialState, const std::string& name)
    {
        return std::unique_ptr<D3D12Buffer>(new D3D12Buffer(this, _allocator.Get(), description, initialState, name));
    }

    std::shared_ptr<rhi::Buffer> D3D12Device::CreateBuffer(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<D3D12Buffer>(new D3D12Buffer(this, D3D12Cast<ID3D12Resource>(nativePtr), name));
    }

    std::shared_ptr<rhi::Texture> D3D12Device::CreateTexture(const rhi::TextureDescription& description, ResourceState initialState, const std::string& name)
    {
        return std::unique_ptr<D3D12Texture>(new D3D12Texture(this, _allocator.Get(), description, initialState, name));
    }

    std::shared_ptr<rhi::Texture> D3D12Device::CreateTexture(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<D3D12Texture>(new D3D12Texture(this, D3D12Cast<ID3D12Resource>(nativePtr), name));
    }

    std::unique_ptr<rhi::CommandList> D3D12Device::CreateCommandList(rhi::CommandListType type, const std::string& name)
    {
        return std::unique_ptr<D3D12CommandList>(new D3D12CommandList(this, type, name));
    }

    std::unique_ptr<CommandListPool> D3D12Device::CreateCommandListPool()
    {
        return std::unique_ptr<D3D12CommandListPool>(new D3D12CommandListPool(this));
    }

    std::unique_ptr<rhi::DescriptorHeap> D3D12Device::CreateDescriptorHeap(const rhi::DescriptorHeapDescription& description, const std::string& name)
    {
        return std::unique_ptr<D3D12DescriptorHeap>(new D3D12DescriptorHeap(this, description, name));
    }

    std::unique_ptr<rhi::QueryHeap> D3D12Device::CreateQueryHeap(const rhi::QueryHeapDescription& description, const std::string& name)
    {
        return std::unique_ptr<D3D12QueryHeap>(new D3D12QueryHeap(this, description, name));
    }

    std::unique_ptr<rhi::Fence> D3D12Device::CreateFence(std::uint64_t initialValue)
    {
        return std::unique_ptr<D3D12Fence>(new D3D12Fence(this, initialValue));
    }

    std::unique_ptr<rhi::StatisticsQuery> D3D12Device::CreateStatisticsQuery(const std::string& name)
    {
        return std::unique_ptr<D3D12StatisticsQuery>(new D3D12StatisticsQuery(this, name));
    }

    std::unique_ptr<rhi::TimestampQuery> D3D12Device::CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name)
    {
        return std::unique_ptr<D3D12TimestampQuery>(new D3D12TimestampQuery(this, timestampsCount, name));
    }

    std::unique_ptr<rhi::CommandSignature> D3D12Device::CreateCommandSignature(const std::vector<rhi::IndirectArgumentDescription>& arguments, rhi::PipelineState* pipelineState, const std::string& name)
    {
        return std::unique_ptr<D3D12CommandSignature>(new D3D12CommandSignature(this, arguments, pipelineState, name));
    }

    std::unique_ptr<rhi::SwapChain> D3D12Device::CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
    {
        return std::unique_ptr<D3D12SwapChain>(new D3D12SwapChain(this, (HWND)windowHandle, width, height, vSync));
    }

    std::unique_ptr<rhi::PipelineState> D3D12Device::CreatePipelineState(const std::string& filepath)
    {
        return std::unique_ptr<D3D12PipelineState>(new D3D12PipelineState(this, filepath));
    }

    void D3D12Device::CreateBufferView(const rhi::BufferView& view, CPUDescriptor& descriptor)
    {
        switch (view.GetType())
        {
        case ResourceViewType::CBV:
            CreateBufferCBV(view, descriptor);
            break;
        case ResourceViewType::SRV:
            CreateBufferSRV(view, descriptor);
            break;
        case ResourceViewType::UAV:
            CreateBufferUAV(view, descriptor);
            break;
        default:
            UNREACHABLE("Unsupported buffer view type.");
            break;
        }
    }

    void D3D12Device::CreateBufferSRV(std::shared_ptr<Buffer> buffer, CPUDescriptor& descriptor)
    {
        BufferView view(buffer.get(),
            ResourceViewType::SRV,
            buffer->GetSize(),
            0,
            0,
            buffer->GetElementCount(),
            buffer->GetStride());

        CreateBufferSRV(view, descriptor);
    }

    void D3D12Device::CreateBufferCBV(std::shared_ptr<Buffer> buffer, CPUDescriptor& descriptor)
    {
        BufferView view(buffer.get(),
            ResourceViewType::CBV,
            buffer->GetSize(),
            0,
            0,
            buffer->GetElementCount(),
            buffer->GetStride());

        CreateBufferCBV(view, descriptor);
    }

    void D3D12Device::CreateBufferUAV(std::shared_ptr<Buffer> buffer, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource)
    {
        BufferView view(buffer.get(),
            ResourceViewType::UAV,
            buffer->GetSize(),
            0,
            0,
            buffer->GetElementCount(),
            buffer->GetStride(),
            counterResource.get());

        CreateBufferUAV(view, descriptor);
    }

    void D3D12Device::CreateTextureView(const rhi::TextureView& view, CPUDescriptor& descriptor)
    {
        switch (view.GetType())
        {
        case ResourceViewType::RTV:
            CreateTextureRTV(view, descriptor);
            break;
        case ResourceViewType::DSV:
            CreateTextureDSV(view, descriptor);
            break;
        case ResourceViewType::SRV:
            CreateTextureSRV(view, descriptor);
            break;
        case ResourceViewType::UAV:
            CreateTextureUAV(view, descriptor);
            break;
        default:
            UNREACHABLE("Unsupported texture view type.");
            break;
        }
    }

    void D3D12Device::CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(), 
            ResourceViewType::RTV, 
            texture->GetDescription().Dimension, 
            0, 
            texture->GetMipLevels(), 
            0, 
            0, 
            texture->GetDepthOrArraySize());

        CreateTextureRTV(view, descriptor);
    }

    void D3D12Device::CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::DSV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureDSV(view, descriptor);
    }

    void D3D12Device::CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::SRV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureSRV(view, descriptor);
    }

    void D3D12Device::CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::UAV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureUAV(view, descriptor);
    }

    std::uint32_t D3D12Device::GetDescriptorHandleIncrementSize(rhi::DescriptorHeapType type) const
    {
        switch (type)
        {
        case rhi::DescriptorHeapType::RTV:
            return _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        case rhi::DescriptorHeapType::DSV:
            return _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        case rhi::DescriptorHeapType::CBV_SRV_UAV:
            return _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        default:
            UNREACHABLE("Unsupported descriptor heap type.");
            return _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
    }

    rhi::AllocationInfo D3D12Device::GetAllocationInfo(const BufferDescription& description) const
    {
        D3D12_RESOURCE_DESC desc = GetD3D12ResourceDesc(description);
        D3D12_RESOURCE_ALLOCATION_INFO allocation = _device->GetResourceAllocationInfo(0, 1, &desc);

        rhi::AllocationInfo allocationInfo =
        {
            .SizeInBytes = allocation.SizeInBytes,
            .Alignment = allocation.Alignment
        };
        return allocationInfo;
    }

    rhi::AllocationInfo D3D12Device::GetAllocationInfo(const rhi::TextureDescription& description) const
    {
        D3D12_RESOURCE_DESC desc = GetD3D12ResourceDesc(description);
        D3D12_RESOURCE_ALLOCATION_INFO allocation = _device->GetResourceAllocationInfo(0, 1, &desc);

        rhi::AllocationInfo allocationInfo =
        {
            .SizeInBytes = allocation.SizeInBytes,
            .Alignment = allocation.Alignment
        };
        return allocationInfo;
    }

    const AdapterInfo& D3D12Device::QueryAdapterInfo()
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
        _adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

        _adapterInfo.VideoMemory =
        {
            .Budget = memoryInfo.Budget,
            .CurrentUsage = memoryInfo.CurrentUsage,
            .AvailableForReservation = memoryInfo.AvailableForReservation,
            .CurrentReservation = memoryInfo.CurrentReservation
        };

        return _adapterInfo;
    }

    tracking::IGPUCrashTracker* D3D12Device::GetCrashTracker()
    {
        return _crashTracker.get();
    }

    void* D3D12Device::GetNative() const
    {
        return static_cast<void*>(_device.Get());
    }

    void D3D12Device::CreateAdapter(bool useWarp)
    {
        ComPtr<IDXGIFactory4> dxgiFactory;
        UINT createFactoryFlags = 0;
#if ENABLE_DEVICE_DEBUG
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif // ENABLE_DEVICE_DEBUG

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
                        D3D_FEATURE_LEVEL_12_0, __uuidof(NativeDevice), nullptr)) &&
                    dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
                {
                    maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                    HRESULT result = dxgiAdapter1.As(&dxgiAdapter4);
                    CHECK(result, "Failed to cast adapter to IDXGIAdapter4.");
                }
            }
        }

        _adapter = dxgiAdapter4;

        DXGI_ADAPTER_DESC adapterDesc;
        HRESULT result = _adapter->GetDesc(&adapterDesc);
        CHECK(result, "Failed to get adapter description.");

        _adapterInfo.Name = ConvertWCharToString(adapterDesc.Description);
        _adapterInfo.Vendor = std::to_string(adapterDesc.VendorId);
        _adapterInfo.DeviceId = std::to_string(adapterDesc.DeviceId);

        LOG_INFO("D3D12 Adapter created.");
    }

    void D3D12Device::CreateDevice()
    {
        _crashTracker->Enable();

        HRESULT createDeviceResult = D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device));
        CHECK(createDeviceResult, "Failed to create D3D12 device.");
        _device->SetName(L"D3D12  Device");

        _crashTracker->Initialize(this);

        // Enable debug messages in debug mode.
#if ENABLE_DEVICE_DEBUG
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
            D3D12_MESSAGE_ID DenyIds[] =
            {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_NON_OPTIMAL_BARRIER_ONLY_EXECUTE_COMMAND_LISTS
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
#endif // ENABLE_DEVICE_DEBUG
    }

    void D3D12Device::CreateAllocator()
    {
        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = _device.Get();
        allocatorDesc.pAdapter = _adapter.Get();

        HRESULT result = D3D12MA::CreateAllocator(&allocatorDesc, &_allocator);
        CHECK(result, "Failed to create D3D12MA allocator.");

        LOG_INFO("D3D12MA allocator created.");
    }

    void D3D12Device::CreateQueues()
    {
        _queueGraphics = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Graphics, "D3D12 Direct Queue"));
        _queueCompute = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Compute, "D3D12 Compute Queue"));
        _queueCopy = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Copy, "D3D12 Copy Queue"));

        LOG_INFO("DX12 Command Queues created.");
    }

    void D3D12Device::CheckFeatureSupport()
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};

        HRESULT result = _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12));
        CHECK(result, "Failed to check D3D12 options 12.");

        _enhancedBarriersSupported = options12.EnhancedBarriersSupported;
    }

    void D3D12Device::CreateBufferSRV(const BufferView& view, CPUDescriptor& descriptor)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvView =
        {
            .Format = GetDXGIFormat(view.GetFormat()),
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer =
                {
                    .FirstElement = view.GetFirstElement(),
                    .NumElements = view.GetNumElements(),
                    .StructureByteStride = view.GetStride(),
                    .Flags = D3D12_BUFFER_SRV_FLAG_NONE
                }
        };

        _device->CreateShaderResourceView(D3D12Cast<ID3D12Resource>(view.GetBuffer()->GetNative()), &srvView, ToD3D12Handle(descriptor));
    }
    
    void D3D12Device::CreateBufferCBV(const BufferView& view, CPUDescriptor& descriptor)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvView =
        {
            .BufferLocation = view.GetVirtualAddress(),
            .SizeInBytes = view.GetSize()
        };

        _device->CreateConstantBufferView(&cbvView, ToD3D12Handle(descriptor));
    }
    
    void D3D12Device::CreateBufferUAV(const BufferView& view, CPUDescriptor& descriptor)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavView =
        {
            .Format = GetDXGIFormat(view.GetFormat()),
            .ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
            .Buffer =
                {
                    .NumElements = view.GetNumElements(),
                    .StructureByteStride = view.GetStride(),
                    .CounterOffsetInBytes = view.GetUAVCounterOffset()
                }
        };

        ID3D12Resource* nativeResource = D3D12Cast<ID3D12Resource>(view.GetBuffer()->GetNative());
        if (view.GetCounterResource())
        {
            ID3D12Resource* nativeCounterResource = view.GetCounterResource() ? D3D12Cast<ID3D12Resource>(view.GetCounterResource()->GetNative()) : nativeResource;
            _device->CreateUnorderedAccessView(nativeResource, nativeCounterResource, &uavView, ToD3D12Handle(descriptor));
        }
        else
        {
            _device->CreateUnorderedAccessView(nativeResource, nullptr, &uavView, ToD3D12Handle(descriptor));
        }
    }
    
    void D3D12Device::CreateTextureRTV(const TextureView& view, CPUDescriptor& descriptor)
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvView =
        {
            .Format = GetDXGIFormat(view.GetFormat())
        };

        if (view.GetDimension() == TextureDimension::Texture1D)
        {
            if (view.GetArraySize() == 0)
            {
                rtvView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                rtvView.Texture1D.MipSlice = view.GetMipSlice();
            }
            else
            {
                rtvView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                rtvView.Texture1DArray.MipSlice = view.GetMipSlice();
                rtvView.Texture1DArray.FirstArraySlice = view.GetFirstArraySlice();
                rtvView.Texture1DArray.ArraySize = view.GetArraySize();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture2D)
        {
            if (view.GetArraySize() == 0)
            {
                rtvView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvView.Texture2D.MipSlice = view.GetMipSlice();
                rtvView.Texture2D.PlaneSlice = view.GetPlaneSlice();
            }
            else
            {
                rtvView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvView.Texture2DArray.MipSlice = view.GetMipSlice();
                rtvView.Texture2DArray.FirstArraySlice = view.GetFirstArraySlice();
                rtvView.Texture2DArray.ArraySize = view.GetArraySize();
                rtvView.Texture2DArray.PlaneSlice = view.GetPlaneSlice();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture3D)
        {
            rtvView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvView.Texture3D.MipSlice = view.GetMipSlice();
            rtvView.Texture3D.FirstWSlice = view.GetFirstArraySlice();
            rtvView.Texture3D.WSize = view.GetArraySize();
        }
        else
        {
            UNREACHABLE("Unsupported texture dimension.");
        }

        _device->CreateRenderTargetView(D3D12Cast<ID3D12Resource>(view.GetTexture()->GetNative()), &rtvView, ToD3D12Handle(descriptor));
    }
    
    void D3D12Device::CreateTextureDSV(const TextureView& view, CPUDescriptor& descriptor)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvView = 
        {
            .Format = GetDXGIFormat(view.GetFormat())
        };

        if (view.GetDimension() == TextureDimension::Texture1D)
        {
            if (view.GetArraySize() == 0)
            {
                dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                dsvView.Texture1D.MipSlice = view.GetMipSlice();
            }
            else
            {
                dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                dsvView.Texture1DArray.MipSlice = view.GetMipSlice();
                dsvView.Texture1DArray.FirstArraySlice = view.GetFirstArraySlice();
                dsvView.Texture1DArray.ArraySize = view.GetArraySize();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture2D)
        {
            if (view.GetArraySize() == 0)
            {
                dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvView.Texture2D.MipSlice = view.GetMipSlice();
            }
            else
            {
                dsvView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                dsvView.Texture2DArray.MipSlice = view.GetMipSlice();
                dsvView.Texture2DArray.FirstArraySlice = view.GetFirstArraySlice();
                dsvView.Texture2DArray.ArraySize = view.GetArraySize();
            }
        }
        else
        {
            UNREACHABLE("Unsupported texture dimension for DSV.");
        }

        _device->CreateDepthStencilView(D3D12Cast<ID3D12Resource>(view.GetTexture()->GetNative()), &dsvView, ToD3D12Handle(descriptor));
    }
    
    void D3D12Device::CreateTextureSRV(const TextureView& view, CPUDescriptor& descriptor)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvView = 
        {
            .Format = GetDXGIFormat(view.GetFormat()),
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
        };

        if (view.GetFormat() == Format::D32_FLOAT)
        {
            srvView.Format = DXGI_FORMAT_R32_FLOAT;
        }

        if (view.GetDimension() == TextureDimension::Texture1D)
        {
            if (view.GetArraySize() == 0)
            {
                srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srvView.Texture1D.MipLevels = view.GetMipLevels();
                srvView.Texture1D.MostDetailedMip = view.GetMostDetailedMip();
                srvView.Texture1D.ResourceMinLODClamp = view.GetResourceMinLODClamp();
            }
            else
            {
                srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srvView.Texture1DArray.MipLevels = view.GetMipLevels();
                srvView.Texture1DArray.MostDetailedMip = view.GetMostDetailedMip();
                srvView.Texture1DArray.FirstArraySlice = view.GetFirstArraySlice();
                srvView.Texture1DArray.ArraySize = view.GetArraySize();
                srvView.Texture1DArray.ResourceMinLODClamp = view.GetResourceMinLODClamp();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture2D)
        {
            if (view.GetArraySize() == 0)
            {
                srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvView.Texture2D.MipLevels = view.GetMipLevels();
                srvView.Texture2D.MostDetailedMip = view.GetMostDetailedMip();
                srvView.Texture2D.PlaneSlice = view.GetPlaneSlice();
                srvView.Texture2D.ResourceMinLODClamp = view.GetResourceMinLODClamp();
            }
            else if (view.GetArraySize() == 6)
            {
                srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srvView.TextureCube.MipLevels = view.GetMipLevels();
                srvView.TextureCube.MostDetailedMip = view.GetMostDetailedMip();
                srvView.TextureCube.ResourceMinLODClamp = view.GetResourceMinLODClamp();
            }
            else
            {
                srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                srvView.Texture2DArray.MipLevels = view.GetMipLevels();
                srvView.Texture2DArray.MostDetailedMip = view.GetMostDetailedMip();
                srvView.Texture2DArray.FirstArraySlice = view.GetFirstArraySlice();
                srvView.Texture2DArray.ArraySize = view.GetArraySize();
                srvView.Texture2DArray.PlaneSlice = view.GetPlaneSlice();
                srvView.Texture2DArray.ResourceMinLODClamp = view.GetResourceMinLODClamp();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture3D)
        {
            srvView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srvView.Texture3D.MipLevels = view.GetMipLevels();
            srvView.Texture3D.MostDetailedMip = view.GetMostDetailedMip();
            srvView.Texture3D.ResourceMinLODClamp = view.GetResourceMinLODClamp();
        }
        else
        {
            UNREACHABLE("Unsupported texture dimension.");
        }

        _device->CreateShaderResourceView(D3D12Cast<ID3D12Resource>(view.GetTexture()->GetNative()), &srvView, ToD3D12Handle(descriptor));
    }
    
    void D3D12Device::CreateTextureUAV(const TextureView& view, CPUDescriptor& descriptor)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavView = 
        {
            .Format = GetDXGIFormat(view.GetFormat())
        };

        if (view.GetDimension() == TextureDimension::Texture1D)
        {
            if (view.GetArraySize() == 0)
            {
                uavView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                uavView.Texture1D.MipSlice = view.GetMipSlice();
            }
            else
            {
                uavView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                uavView.Texture1DArray.MipSlice = view.GetMipSlice();
                uavView.Texture1DArray.FirstArraySlice = view.GetFirstArraySlice();
                uavView.Texture1DArray.ArraySize = view.GetArraySize();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture2D)
        {
            if (view.GetArraySize() == 0)
            {
                uavView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uavView.Texture2D.MipSlice = view.GetMipSlice();
                uavView.Texture2D.PlaneSlice = view.GetPlaneSlice();
            }
            else
            {
                uavView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                uavView.Texture2DArray.MipSlice = view.GetMipSlice();
                uavView.Texture2DArray.FirstArraySlice = view.GetFirstArraySlice();
                uavView.Texture2DArray.ArraySize = view.GetArraySize();
                uavView.Texture2DArray.PlaneSlice = view.GetPlaneSlice();
            }
        }
        else if (view.GetDimension() == TextureDimension::Texture3D)
        {
            uavView.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            uavView.Texture3D.MipSlice = view.GetMipSlice();
            uavView.Texture3D.FirstWSlice = view.GetFirstArraySlice();
            uavView.Texture3D.WSize = view.GetArraySize();
        }
        else
        {
            UNREACHABLE("Unsupported texture dimension for UAV.");
        }

        _device->CreateUnorderedAccessView(D3D12Cast<ID3D12Resource>(view.GetTexture()->GetNative()), nullptr, &uavView, ToD3D12Handle(descriptor));
    }
} // namespace rhi::d3d12
