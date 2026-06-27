
#include "RHI_PCH.h"

#include "D3D12Texture.h"

namespace rhi::d3d12
{
    D3D12Texture::D3D12Texture(Device* device, D3D12MA::Allocator* allocator, const TextureDescription& description, ResourceState initialState, const std::string& name)
        : _description(description)
        , _resource(device, allocator, description, initialState, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12Texture::D3D12Texture(Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name)
        : _resource(device, nativeTexturePtr, name)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_RESOURCE_DESC d3d12desc = nativeTexturePtr->GetDesc();

        _description =
        {
            .Width = static_cast<std::uint32_t>(d3d12desc.Width),
            .Height = d3d12desc.Height,
            .DepthOrArraySize = d3d12desc.DepthOrArraySize,
            .MipLevels = d3d12desc.MipLevels,
            .InitialState = _resource.GetInitialState()
        };

        switch (d3d12desc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                _description.Dimension = TextureDimension::Texture1D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                _description.Dimension = TextureDimension::Texture2D;
                break;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                _description.Dimension = TextureDimension::Texture3D;
                break;
            default:
                UNREACHABLE("Unsupported D3D12 resource dimension.");
                _description.Dimension = TextureDimension::Unknown;
                break;
        }
    }

    D3D12Texture::D3D12Texture(D3D12Texture&& other) noexcept
        : Texture(std::move(other))
        , _description(std::move(other._description))
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
            Texture::operator=(std::move(other));
            _description = std::move(other._description);
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

    void D3D12Texture::SetCurrentState(ResourceState state)
    {
        _resource.SetCurrentState(state);
    }

    const TextureDescription& D3D12Texture::GetDescription() const
    {
        return _description;
    }

    std::uint32_t D3D12Texture::GetWidth() const
    {
        return _description.Width;
    }

    std::uint32_t D3D12Texture::GetHeight() const
    {
        return _description.Height;
    }

    std::uint32_t D3D12Texture::GetMipLevels() const
    {
        return _description.MipLevels;
    }

    std::uint32_t D3D12Texture::GetDepthOrArraySize()
    {
        return _description.DepthOrArraySize;
    }

    Format D3D12Texture::GetFormat() const
    {
        return _description.Format;
    }

    TextureDimension D3D12Texture::GetDimension() const
    {
        return TextureDimension();
    }

    const ResourceID& D3D12Texture::GetID() const
    {
        return _resource.GetID();
    }

    void* D3D12Texture::GetNative() const
    {
        return _resource.GetNative();
    }
} // namespace rhi::d3d12
