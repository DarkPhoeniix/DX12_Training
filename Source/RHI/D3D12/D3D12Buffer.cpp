
#include "RHI_PCH.h"

#include "D3D12Buffer.h"

namespace rhi::d3d12
{
    D3D12Buffer::D3D12Buffer(rhi::Device* device, const rhi::BufferDescription& description, const void* initialData, const std::string& name)
        : _resource(device, description, initialData, name)
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

    std::uint64_t D3D12Buffer::GetVirtualAddress()
    {
        return static_cast<std::uint64_t>(_resource.GetVirtualAddress());
    }

    ResourceState D3D12Buffer::GetInitialState() const
    {
        return _resource.GetInitialState();
    }

    ResourceState D3D12Buffer::GetCurrentState() const
    {
        return _resource.GetCurrentState();
    }

    const AllocationInfo& D3D12Buffer::GetAllocationInfo() const
    {
        return _resource.GetAllocationInfo();
    }

    void* D3D12Buffer::GetNative() const
    {
        return _resource.GetNative();
    }
} // namespace rhi::d3d12
