
#include "RHI_PCH.h"

#include "D3D12Device.h"

#include "D3D12Buffer.h"
#include "D3D12CommandList.h"
#include "D3D12CommandQueue.h"
#include "D3D12DescriptorHeap.h"
#include "D3D12Descriptor.h"
#include "D3D12Fence.h"
#include "D3D12Heap.h"
#include "D3D12QueryHeap.h"
#include "D3D12PipelineState.h"
#include "D3D12SwapChain.h"
#include "D3D12Texture.h"
#include "IGPUCrashTracker.h"
#include "D3D12StatisticsQuery.h"
#include "D3D12TimestampQuery.h"

namespace rhi::d3d12
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

    D3D12Device::D3D12Device()
        : _crashTracker(tracking::IGPUCrashTracker::Create())
    {
#if defined(_DEBUG)
        EnableDXDebugLayer();
#endif

        CreateAdapter();
        CreateDevice();
        CreateQueues();

        CheckFeatureSupport();

        LOG_INFO("DX12 device initialized.");
    }

    D3D12Device::~D3D12Device()
    {
        _adapter = nullptr;
        _device = nullptr;

        LOG_INFO("DX12 device destroyed.");
    }

    D3D12Device::D3D12Device(D3D12Device&& other) noexcept
        : _device(std::move(other._device))
        , _adapter(std::move(other._adapter))
        , _queueGraphics(std::move(other._queueGraphics))
        , _queueCompute(std::move(other._queueCompute))
        , _queueCopy(std::move(other._queueCopy))
        , _swapChain(std::move(other._swapChain))
    {
    }

    D3D12Device& D3D12Device::operator=(D3D12Device&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _adapter = std::move(other._adapter);
            _queueGraphics = std::move(other._queueGraphics);
            _queueCompute = std::move(other._queueCompute);
            _queueCopy = std::move(other._queueCopy);
            _swapChain = std::move(other._swapChain);
        }

        return *this;
    }

    bool D3D12Device::IsEnhancedBarriersSupported()
    {
        return _enhancedBarriersSupported;
    }

    void D3D12Device::BindSwapChain(rhi::SwapChain& swapChain)
    {
        _swapChain = &swapChain;
    }

    rhi::CommandQueue* D3D12Device::GetStreamQueue()
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

    //std::shared_ptr<rhi::Resource> D3D12Device::GetBackBuffer()
    //{
    //    return _swapChain->GetBackBuffer();
    //}

    void D3D12Device::Present()
    {
        _swapChain->Present();
    }

    std::unique_ptr<CommandList> D3D12Device::CreateCommandList(CommandListType type)
    {
        return std::unique_ptr<D3D12CommandList>(new D3D12CommandList(this, type));
    }

    std::unique_ptr<DescriptorHeap> D3D12Device::CreateDescriptorHeap(const DescriptorHeapDescription& description)
    {
        return std::unique_ptr<D3D12DescriptorHeap>(new D3D12DescriptorHeap(this, description));
    }

    std::shared_ptr<Buffer> D3D12Device::CreateBuffer(const BufferDescription& description, const void* initialData)
    {
        return std::unique_ptr<D3D12Buffer>(new D3D12Buffer(this, description, initialData));
    }

    std::shared_ptr<Buffer> D3D12Device::CreateBuffer(void* nativePtr)
    {
        return std::unique_ptr<D3D12Buffer>(new D3D12Buffer(this, D3D12Cast<ID3D12Resource>(nativePtr)));
    }

    std::shared_ptr<Texture> D3D12Device::CreateTexture(const TextureDescription& description, const void* initialData)
    {
        return std::unique_ptr<D3D12Texture>(new D3D12Texture(this, description, initialData));
    }

    std::shared_ptr<Texture> D3D12Device::CreateTexture(void* nativePtr)
    {
        return std::unique_ptr<D3D12Texture>(new D3D12Texture(this, D3D12Cast<ID3D12Resource>(nativePtr)));
    }

    std::unique_ptr<QueryHeap> D3D12Device::CreateQueryHeap(const QueryHeapDescription& description)
    {
        return std::unique_ptr<D3D12QueryHeap>(new D3D12QueryHeap(this, description));
    }

    std::unique_ptr<Fence> D3D12Device::CreateFence(std::uint64_t initialValue)
    {
        return std::unique_ptr<D3D12Fence>(new D3D12Fence(this, initialValue));
    }

    std::unique_ptr<Heap> D3D12Device::CreateHeap(const HeapDescription& description)
    {
        return std::unique_ptr<D3D12Heap>(new D3D12Heap(this, description));
    }

    std::unique_ptr<StatisticsQuery> D3D12Device::CreateStatisticsQuery()
    {
        return std::unique_ptr<D3D12StatisticsQuery>(new D3D12StatisticsQuery(this));
    }

    std::unique_ptr<TimestampQuery> D3D12Device::CreateTimestampQuery(std::uint32_t timestampsCount)
    {
        return std::unique_ptr<D3D12TimestampQuery>(new D3D12TimestampQuery(this, timestampsCount));
    }

    void D3D12Device::CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
    }

    void D3D12Device::CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
    }

    void D3D12Device::CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource)
    {
    }

    void D3D12Device::CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        D3D12_RENDER_TARGET_VIEW_DESC desc =
        {
            .Format = GetDXGIFormat(texture->GetFormat()),
            .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
            .Texture2D = {}
        };

        _device->CreateRenderTargetView(D3D12Cast<ID3D12Resource>(texture->GetNative()), &desc, ToD3D12Handle(descriptor));
    }

    void D3D12Device::CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12Device::CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12Device::CreateTextureCBV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void D3D12Device::CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource)
    {
        NOT_IMPLEMENTED();
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

    //void D3D12Device::CreateRenderTargetView(const rhi::RenderTargetView& view, rhi::DescriptorHeap& descriptorHeap)
    //{
    //    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
    //    _device->CreateRenderTargetView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    //}

    //void D3D12Device::CreateDepthStencilView(const rhi::DepthStencilView& view, rhi::DescriptorHeap& descriptorHeap)
    //{
    //    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
    //    _device->CreateDepthStencilView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    //}

    //void D3D12Device::CreateConstantBufferView(const rhi::ConstantBufferView& view, rhi::DescriptorHeap& descriptorHeap)
    //{
    //    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
    //    _device->CreateConstantBufferView(&view, heapHandle);
    //}

    //void D3D12Device::CreateShaderResourceView(const rhi::ShaderResourceView& view, rhi::DescriptorHeap& descriptorHeap)
    //{
    //    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
    //    _device->CreateShaderResourceView(view.Owner->GetDXResource().Get(), &view, heapHandle);
    //}

    //void D3D12Device::CreateUnorderedAccessView(const rhi::UnorderedAccessView& view, rhi::DescriptorHeap& descriptorHeap, std::shared_ptr<rhi::Resource> counterResource)
    //{
    //    ID3D12Resource* counter = counterResource ? counterResource->GetDXResource().Get() : nullptr;
    //    D3D12_CPU_DESCRIPTOR_HANDLE heapHandle = descriptorHeap.GetCPUHandleWithOffset(descriptorHeap.Offset());
    //    _device->CreateUnorderedAccessView(view.Owner->GetDXResource().Get(), counter, &view, heapHandle);
    //}

    //void D3D12Device::CreateRenderTargetView(const rhi::RenderTargetView& view, rhi::Descriptor descriptor)
    //{
    //    _device->CreateRenderTargetView(view.Owner->GetDXResource().Get(), &view, descriptor);
    //}

    //void D3D12Device::CreateDepthStencilView(const rhi::DepthStencilView& view, rhi::Descriptor descriptor)
    //{
    //    _device->CreateDepthStencilView(view.Owner->GetDXResource().Get(), &view, descriptor);
    //}

    //void D3D12Device::CreateConstantBufferView(const rhi::ConstantBufferView& view, rhi::Descriptor descriptor)
    //{
    //    _device->CreateConstantBufferView(&view, descriptor);
    //}

    //void D3D12Device::CreateShaderResourceView(const rhi::ShaderResourceView& view, rhi::Descriptor descriptor)
    //{
    //    _device->CreateShaderResourceView(view.Owner->GetDXResource().Get(), &view, descriptor);
    //}

    //void D3D12Device::CreateUnorderedAccessView(const rhi::UnorderedAccessView& view, rhi::Descriptor descriptor, std::shared_ptr<rhi::Resource> counterResource)
    //{
    //    ID3D12Resource* counter = view.Owner->GetResourceDescription().GetUAVCounterOffset() != std::uint32_t(-1) ? view.Owner->GetDXResource().Get() : nullptr; // TODO: Use counterResource if provided
    //    _device->CreateUnorderedAccessView(view.Owner->GetDXResource().Get(), counter, &view, descriptor);
    //}

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

        LOG_INFO("D3D12 Adapter created.");
    }

    void D3D12Device::CreateDevice()
    {
        _crashTracker->Enable();

        HRESULT createDeviceResult = D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)); // TODO: upgrade level?
        CHECK(createDeviceResult, "Failed to create D3D12 device.");
        _device->SetName(L"D3D12  Device");

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
#endif
    }

    void D3D12Device::CreateQueues()
    {
        _queueGraphics = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Graphics, "D3D12 Direct Queue"));
        _queueCompute = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Graphics, "D3D12 Compute Queue"));
        _queueCopy = std::unique_ptr<D3D12CommandQueue>(new D3D12CommandQueue(this, rhi::CommandListType::Graphics, "D3D12 Copy Queue"));

        LOG_INFO("DX12 Command Queues created.");
    }

    void D3D12Device::CheckFeatureSupport()
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};

        HRESULT result = _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12));
        CHECK(result, "Failed to check D3D12 options 12.");

        _enhancedBarriersSupported = options12.EnhancedBarriersSupported;
    }
} // namespace rhi::d3d12
