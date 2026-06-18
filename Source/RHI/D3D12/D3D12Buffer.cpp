
#include "RHI_PCH.h"

#include "D3D12Buffer.h"

namespace rhi::d3d12
{
    D3D12Buffer::D3D12Buffer(rhi::Device* device, D3D12MA::Allocator* allocator, const rhi::BufferDescription& description, ResourceState initialState, const std::string& name)
        : _description(description)
        , _resource(device, allocator, description, initialState, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Buffer::D3D12Buffer(rhi::Device* device, const rhi::BufferDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState, const std::string& name)
        : _description(description)
        , _resource(device, description, heap, offset, initialState, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Buffer::D3D12Buffer(rhi::Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name)
        : _resource(device, nativeTexturePtr, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Buffer::D3D12Buffer(D3D12Buffer&& other) noexcept
        : rhi::Buffer(std::move(other))
        , _description(std::move(other._description))
        , _resource(std::move(other._resource))
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Buffer::~D3D12Buffer()
    {
    }

    D3D12Buffer& D3D12Buffer::operator=(D3D12Buffer&& other) noexcept
    {
        if (this != &other)
        {
            rhi::Buffer::operator=(std::move(other));
            _description = std::move(other._description);
            _resource = std::move(_resource);
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    void* D3D12Buffer::Map(std::uint32_t begin, std::uint32_t end)
    {
        return _resource.Map(begin, end);
    }

    void D3D12Buffer::Unmap()
    {
        _resource.Unmap();
    }

    std::uint64_t D3D12Buffer::GetVirtualAddress(std::uint64_t offset)
    {
        return static_cast<std::uint64_t>(_resource.GetVirtualAddress()) + offset;
    }

    ResourceState D3D12Buffer::GetInitialState() const
    {
        return _resource.GetInitialState();
    }

    ResourceState D3D12Buffer::GetCurrentState() const
    {
        return _resource.GetCurrentState();
    }

    void D3D12Buffer::SetCurrentState(ResourceState state)
    {
        _resource.SetCurrentState(state);
    }

    const BufferDescription& D3D12Buffer::GetDescription() const
    {
        return _description;
    }

    std::uint32_t D3D12Buffer::GetSize() const
    {
        return _description.Size;
    }

    std::uint32_t D3D12Buffer::GetStride() const
    {
        return _description.Stride;
    }

    std::uint32_t D3D12Buffer::GetElementCount() const
    {
        return (_description.Stride > 0) ? static_cast<std::uint32_t>(_description.Size / _description.Stride) : 0;
    }

    std::uint32_t D3D12Buffer::GetUAVCounterOffset() const
    {
        return _resource.GetUAVCounterOffset();
    }

    const ResourceID& D3D12Buffer::GetID() const
    {
        return _resource.GetID();
    }

    void* D3D12Buffer::GetNative() const
    {
        return _resource.GetNative();
    }
} // namespace rhi::d3d12
