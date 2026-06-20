
#include "RHI_PCH.h"

#include "D3D12Resource.h"

#include "D3D12Device.h"
#include "D3D12Helpers.h"

#include "ResourceIdGenerator.h"
#include "Buffer.h"
#include "Texture.h"

#include <D3D12MemAlloc.h>

namespace rhi::d3d12
{
    namespace
    {
        D3D12_HEAP_PROPERTIES CreateHeapProperties(ResourceUsage usage)
        {
            D3D12_HEAP_PROPERTIES heapDesc = 
            {
                .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
                .CreationNodeMask = 1,
                .VisibleNodeMask = 1
            };

            switch (usage)
            {
                case ResourceUsage::Upload:
                    heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
                    break;
                case ResourceUsage::GPUUpload:
                    heapDesc.Type = D3D12_HEAP_TYPE_GPU_UPLOAD;
                    break;
                case ResourceUsage::Readback:
                    heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
                    break;
                default:
                    heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
                    break;
            }

            return heapDesc;
        }
    } // namespace unnamed

    D3D12Resource::D3D12Resource(rhi::Device* device, D3D12MA::Allocator* allocator, const BufferDescription& description, ResourceState initialState, const std::string& name)
        : _device(device)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(initialState)
        , _currentState(initialState)
        , _stride(description.Stride)
        , _uavCounterOffset(description.UAVCounterOffset)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_RESOURCE_DESC resourceDesc = GetD3D12ResourceDesc(description);
        D3D12_HEAP_PROPERTIES heapDesc = CreateHeapProperties(description.Usage);
        D3D12_CLEAR_VALUE* pClearValue = nullptr;

        CreateCommitedResource(allocator, resourceDesc, heapDesc, pClearValue);
    }

    D3D12Resource::D3D12Resource(rhi::Device* device, D3D12MA::Allocator* allocator, const TextureDescription& description, ResourceState initialState, const std::string& name)
        : _device(device)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(initialState)
        , _currentState(initialState)
        , _stride(0)
        , _uavCounterOffset(static_cast<std::uint32_t>(-1))
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_RESOURCE_DESC resourceDesc = GetD3D12ResourceDesc(description);
        D3D12_HEAP_PROPERTIES heapDesc = CreateHeapProperties(description.Usage);
        D3D12_CLEAR_VALUE* pClearValue = nullptr;
        D3D12_CLEAR_VALUE clearValue;

        if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {
            clearValue.Format = GetDXGIFormat(description.Format);
            clearValue.Color[0] = description.ClearValue.Color.R;
            clearValue.Color[1] = description.ClearValue.Color.G;
            clearValue.Color[2] = description.ClearValue.Color.B;
            clearValue.Color[3] = description.ClearValue.Color.A;

            pClearValue = &clearValue;
        }
        else if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        {
            clearValue.Format = GetDXGIFormat(description.Format);
            clearValue.DepthStencil.Depth = description.ClearValue.DepthStencil.Depth;
            clearValue.DepthStencil.Stencil = description.ClearValue.DepthStencil.Stencil;

            pClearValue = &clearValue;
        }

        CreateCommitedResource(allocator, resourceDesc, heapDesc, pClearValue);
    }

    D3D12Resource::D3D12Resource(rhi::Device* device, ID3D12Resource* resource, const std::string& name)
        : _device(device)
        , _resource(resource)
        , _description(resource->GetDesc())
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(rhi::ResourceState::Common)
        , _currentState(rhi::ResourceState::Common)
        , _stride(0)
        , _uavCounterOffset(static_cast<std::uint32_t>(-1))
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Resource::~D3D12Resource()
    {
        _allocation.Reset();
        _resource.Reset();
    }

    D3D12Resource::D3D12Resource(D3D12Resource&& other) noexcept
        : _device(std::move(other._device))
        , _resource(std::move(other._resource))
        , _allocation(std::move(other._allocation))
        , _description(std::move(other._description))
        , _ID(std::move(other._ID))
        , _initialState(std::move(other._initialState))
        , _currentState(std::move(other._currentState))
        , _stride(other._stride)
        , _uavCounterOffset(other._uavCounterOffset)
#if ENABLE_DEBUG_NAMES
        , _name(other._name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Resource& D3D12Resource::operator=(D3D12Resource&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _resource = std::move(other._resource);
            _allocation = std::move(other._allocation);
            _description = std::move(other._description);
            _ID = std::move(other._ID);
            _initialState = std::move(other._initialState);
            _currentState = std::move(other._currentState);
            _stride = other._stride;
            _uavCounterOffset = other._uavCounterOffset;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    const ResourceID& D3D12Resource::GetID() const
    {
        return _ID;
    }

    void* D3D12Resource::Map(std::uint32_t begin, std::uint32_t end)
    {
        void* data = nullptr;

        D3D12_RANGE range;
        range.Begin = begin;
        range.End = end;
        _resource->Map(0, &range, &data);

        return data;
    }

    void D3D12Resource::Unmap()
    {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = 0;

        _resource->Unmap(0, &range);
    }

    std::uint64_t D3D12Resource::GetVirtualAddress() const
    {
        return _resource->GetGPUVirtualAddress();
    }

    ResourceState D3D12Resource::GetInitialState() const
    {
        return _initialState;
    }

    ResourceState D3D12Resource::GetCurrentState() const
    {
        return _currentState;
    }

    void D3D12Resource::SetCurrentState(ResourceState state)
    {
        _currentState = state;
    }

    std::uint32_t D3D12Resource::GetUAVCounterOffset() const
    {
        return _uavCounterOffset;
    }

    void* D3D12Resource::GetNative() const
    {
        return static_cast<void*>(_resource.Get());
    }

    void D3D12Resource::CreateCommitedResource(D3D12MA::Allocator* allocator, const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperties, D3D12_CLEAR_VALUE* clearValue)
    {
        ASSERT(allocator, "Allocator is nullptr when creating a committed resource.");

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = heapProperties.Type;

        // Render target and depth stencil resources are allocated as committed (their own heap) rather than suballocated.
        // Placed resources have undefined initial contents and the runtime requires them to be initialized with a
        // full-subresource Discard/Clear/Copy before use, whereas committed resources are implicitly initialized.
        if (resourceDesc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
        {
            allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;
        }

        // Create the resource with an enhanced-barrier initial layout so it interops with the engine's
        // enhanced Barrier() transitions. Buffers have no layout and must use UNDEFINED; textures use the
        // layout matching their initial state.
        const bool isBuffer = (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
        const D3D12_BARRIER_LAYOUT initialLayout = isBuffer ? D3D12_BARRIER_LAYOUT_UNDEFINED : GetD3D12Layout(_initialState);

        CD3DX12_RESOURCE_DESC1 resourceDesc1(resourceDesc);

        HRESULT result = allocator->CreateResource3(
            &allocationDesc,
            &resourceDesc1,
            initialLayout,
            clearValue,
            0,
            nullptr,
            &_allocation,
            IID_PPV_ARGS(&_resource));
        CHECK(result, "Failed to create committed resource.");

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_resource.Get(), _name);
#endif // ENABLE_DEBUG_NAMES
    }
} // namespace rhi::d3d12
