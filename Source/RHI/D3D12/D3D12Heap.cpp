
#include "RHI_PCH.h"

#include "D3D12Heap.h"

#include "D3D12Helpers.h"

namespace rhi::d3d12
{
    D3D12Heap::D3D12Heap(rhi::Device* device, const rhi::HeapDescription& description, const std::string& name)
        : _heap(nullptr)
        , _description(description)
        , _resourceOffset(0)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_HEAP_DESC desc =
        {
            .SizeInBytes = description.SizeInBytes,
            .Properties = GetD3D12HeapProperties(description.Properties),
            .Alignment = description.Alignment,
            .Flags = (D3D12_HEAP_FLAGS)description.Flags
        };

        ID3D12Device* d3d12DeviceNative = D3D12Cast<ID3D12Device>(device->GetNative());

        HRESULT result = d3d12DeviceNative->CreateHeap(&desc, IID_PPV_ARGS(&_heap));
        CHECK(result, "Failed to create D3D12Heap.");

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_heap.Get(), name);
#endif // ENABLE_DEBUG_NAMES
    }

    D3D12Heap::D3D12Heap(D3D12Heap&& other) noexcept
        : _heap(std::move(other._heap))
        , _description(std::move(other._description))
        , _resourceOffset(other._resourceOffset)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Heap::~D3D12Heap()
    {
    }

    D3D12Heap& D3D12Heap::operator=(D3D12Heap&& other) noexcept
    {
        if (this != &other)
        {
            _heap = std::move(other._heap);
            _description = std::move(other._description);
            _resourceOffset = other._resourceOffset;
            _device = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

            return *this;
    }

    std::shared_ptr<rhi::Buffer> D3D12Heap::PlaceResource(const rhi::BufferDescription& bufferDesc, ResourceState state, std::uint64_t offset)
    {
        ASSERT(_device, "Device is nullptr when placing resource in a heap.");

        bool isDefaultHeapOffset = (offset == (std::uint64_t)-1);
        if (isDefaultHeapOffset)
        {
            offset = _resourceOffset;
        }

        rhi::AllocationInfo allocationInfo = _device->GetAllocationInfo(bufferDesc);
        std::uint64_t size = allocationInfo.SizeInBytes;
        ASSERT((offset + size) <= _description.SizeInBytes, "Heap is full.");

        std::shared_ptr<rhi::Buffer> buffer = _device->CreateBuffer(bufferDesc, this, offset, state);

        _resourceOffset += Math::AlignUp(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        return buffer;
    }

    std::shared_ptr<rhi::Texture> D3D12Heap::PlaceResource(const rhi::TextureDescription& textureDesc, ResourceState state, std::uint64_t offset)
    {
        ASSERT(_device, "Device is nullptr when placing resource in a heap.");

        bool isDefaultHeapOffset = (offset == (std::uint64_t)-1);
        if (isDefaultHeapOffset)
        {
            offset = _resourceOffset;
        }

        rhi::AllocationInfo allocationInfo = _device->GetAllocationInfo(textureDesc);
        std::uint64_t size = allocationInfo.SizeInBytes;
        ASSERT((offset + size) <= _description.SizeInBytes, "Heap is full.");

        std::shared_ptr<rhi::Texture> texture = _device->CreateTexture(textureDesc, this, offset, state);

        _resourceOffset += Math::AlignUp(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        return texture;
    }

    void D3D12Heap::Reset()
    {
        NOT_IMPLEMENTED();
    }

    void* D3D12Heap::GetNative() const
    {
        return static_cast<void*>(_heap.Get());
    }
} // namespace rhi::d3d12
