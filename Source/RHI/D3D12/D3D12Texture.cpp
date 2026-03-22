
#include "RHI_PCH.h"
#include "D3D12Texture.h"

namespace rhi::d3d12
{
    D3D12Texture::D3D12Texture(rhi::Device* device, const TextureDescription& description, const void* initialData, const std::string& name)
        : _resource(device, description, initialData, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Texture::D3D12Texture(rhi::Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name)
        : _resource(device, nativeTexturePtr, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Texture::D3D12Texture(D3D12Texture&& other)
        : rhi::Texture(std::move(other))
        , _resource(std::move(other._resource))
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Texture& D3D12Texture::operator=(D3D12Texture&& other) noexcept
    {
        if (this != &other)
        {
            rhi::Texture::operator=(std::move(other));
            _resource = std::move(other._resource);
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    void* D3D12Texture::Map(std::uint32_t begin, std::uint32_t end)
    {
        return _resource.Map(begin, end);
    }

    void D3D12Texture::Unmap()
    {
        _resource.Unmap();
    }

    std::uint64_t D3D12Texture::GetVirtualAddress()
    {
        return _resource.GetVirtualAddress();
    }

    ResourceState D3D12Texture::GetInitialState() const
    {
        return _resource.GetInitialState();
    }

    ResourceState D3D12Texture::GetCurrentState() const
    {
        return _resource.GetCurrentState();
    }

    const AllocationInfo& D3D12Texture::GetAllocationInfo() const
    {
        return _resource.GetAllocationInfo();
    }

    void* D3D12Texture::GetNative() const
    {
        return _resource.GetNative();
    }
} // namespace rhi::d3d12
