#include "DX12LibPCH.h"

#include "ResourceDescription.h"

namespace dx12
{
    ResourceDescription::ResourceDescription()
        : _resourceDescription{}
        , _resourceType(ResourceType::None)
        , _stride(0)
        , _clearValue(nullptr)
        , _UAVCounterOffset(std::uint32_t(-1))
    {
        _resourceDescription.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

        _resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;

        _resourceDescription.Height = 1;
        _resourceDescription.DepthOrArraySize = 1;

        _resourceDescription.SampleDesc.Count = 1;
        _resourceDescription.SampleDesc.Quality = 0;
        _resourceDescription.DepthOrArraySize = 1;
        _resourceDescription.MipLevels = 1;

        _resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        _resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    }

    ResourceDescription::ResourceDescription(const D3D12_RESOURCE_DESC& description)
        : _resourceDescription(description)
        , _resourceType(ResourceType::None)
        , _stride(0)
        , _clearValue(nullptr)
        , _UAVCounterOffset(std::uint32_t(-1))
    {
    }

    ResourceDescription::ResourceDescription(const ResourceDescription& other)
        : _resourceDescription(other._resourceDescription)
        , _resourceType(other._resourceType)
        , _stride(other._stride)
        , _clearValue(other._clearValue)
        , _UAVCounterOffset(other._UAVCounterOffset)
    {
    }

    ResourceDescription& ResourceDescription::operator=(const ResourceDescription& other)
    {
        if (this != &other)
        {
            _resourceDescription = other._resourceDescription;
            _resourceType = other._resourceType;
            _stride = other._stride;
            _clearValue = other._clearValue;
            _UAVCounterOffset = other._UAVCounterOffset;
        }

        return *this;
    }

    void ResourceDescription::SetDimension(D3D12_RESOURCE_DIMENSION dimension)
    {
        _resourceDescription.Dimension = dimension;
    }

    D3D12_RESOURCE_DIMENSION ResourceDescription::GetDimension() const
    {
        return _resourceDescription.Dimension;
    }

    void ResourceDescription::SetAlignment(std::uint64_t alignment)
    {
        _resourceDescription.Alignment = alignment;
    }

    std::uint64_t ResourceDescription::GetAlignment() const
    {
        return _resourceDescription.Alignment;
    }

    void ResourceDescription::SetSize(const DirectX::XMUINT2& size)
    {
        _resourceDescription.Width = size.x;
        _resourceDescription.Height = size.y;
    }

    DirectX::XMUINT2 ResourceDescription::GetSize() const
    {
        // TODO: maybe it's wrong
        return { (std::uint32_t)_resourceDescription.Width, (std::uint32_t)_resourceDescription.Height };
    }

    void ResourceDescription::SetDepthOrArraySize(std::uint16_t depthOrArraySize)
    {
        _resourceDescription.DepthOrArraySize = depthOrArraySize;
    }

    std::uint16_t ResourceDescription::GetDepthOrArraySize() const
    {
        return _resourceDescription.DepthOrArraySize;
    }

    void ResourceDescription::SetMipLevels(std::uint16_t mipLevels)
    {
        _resourceDescription.MipLevels = mipLevels;
    }

    std::uint16_t ResourceDescription::GetMipLevels() const
    {
        return _resourceDescription.MipLevels;
    }

    void ResourceDescription::SetFormat(DXGI_FORMAT format)
    {
        _resourceDescription.Format = format;
    }

    DXGI_FORMAT ResourceDescription::GetFormat() const
    {
        return _resourceDescription.Format;
    }

    void ResourceDescription::SetSampleDescription(const DXGI_SAMPLE_DESC& sampleDescription)
    {
        _resourceDescription.SampleDesc = sampleDescription;
    }

    DXGI_SAMPLE_DESC ResourceDescription::GetSampleDescription() const
    {
        return _resourceDescription.SampleDesc;
    }

    void ResourceDescription::SetLayout(D3D12_TEXTURE_LAYOUT textureLayout)
    {
        _resourceDescription.Layout = textureLayout;
    }

    D3D12_TEXTURE_LAYOUT ResourceDescription::GetLayout() const
    {
        return _resourceDescription.Layout;
    }

    void ResourceDescription::SetFlags(D3D12_RESOURCE_FLAGS flags)
    {
        _resourceDescription.Flags = flags;
    }

    void ResourceDescription::AddFlags(D3D12_RESOURCE_FLAGS flags)
    {
        _resourceDescription.Flags |= flags;
    }

    D3D12_RESOURCE_FLAGS ResourceDescription::GetFlags() const
    {
        return _resourceDescription.Flags;
    }

    void ResourceDescription::SetResourceType(ResourceType type)
    {
        _resourceType = type;

        if ((type & ResourceType::Texture) != ResourceType::None)
        {
            _resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            _resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        }
        else if ((type & ResourceType::Buffer) != ResourceType::None)
        {
            _resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        }

        UpdateFlags(_resourceType);
    }

    void ResourceDescription::AddResourceType(ResourceType type)
    {
        _resourceType |= type;
    }

    ResourceType ResourceDescription::GetResourceType() const
    {
        return _resourceType;
    }

    bool ResourceDescription::IsType(ResourceType type) const
    {
        return bool(_resourceType & type);
    }

    void ResourceDescription::SetStride(std::uint32_t stride)
    {
        _stride = stride;
    }

    std::uint32_t ResourceDescription::GetStride() const
    {
        return _stride;
    }

    void ResourceDescription::SetClearValue(D3D12_CLEAR_VALUE clearValue)
    {
        _clearValue = std::make_shared<D3D12_CLEAR_VALUE>(clearValue);
    }

    void ResourceDescription::SetClearValue(const DirectX::XMFLOAT4& clearValue)
    {
        _clearValue->Color[0] = clearValue.x;
        _clearValue->Color[1] = clearValue.y;
        _clearValue->Color[2] = clearValue.z;
        _clearValue->Color[3] = clearValue.w;
    }

    std::shared_ptr<D3D12_CLEAR_VALUE> ResourceDescription::GetClearValue() const
    {
        return _clearValue;
    }

    void ResourceDescription::SetUAVCounterOffset(std::uint32_t offset)
    {
        _UAVCounterOffset = offset;
    }

    std::uint32_t ResourceDescription::GetUAVCounterOffset() const
    {
        return _UAVCounterOffset;
    }

    D3D12_RESOURCE_DESC ResourceDescription::CreateDXResourceDescription() const
    {
        return _resourceDescription;
    }

    void ResourceDescription::UpdateSize(ResourceType type)
    {
        if ((type & ResourceType::Buffer) != ResourceType::None)
        {
            std::uint64_t rowBytes = _resourceDescription.Width;

            if ((type & ResourceType::Aligned) != ResourceType::None)
            {
                // calculate width of buffer
                std::uint64_t aligned = _stride;
                if ((type & ResourceType::Dynamic) != ResourceType::None)
                {
                    aligned = (_stride + 255) & ~255;
                }
                rowBytes *= aligned;
            }
            else
            {
                // calculate width of buffer
                rowBytes *= _stride;
            }

            _resourceDescription.Width = rowBytes;
        }
    }

    void ResourceDescription::UpdateFlags(ResourceType type)
    {
        if ((type & ResourceType::Unordered) != ResourceType::None)
        {
            _resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        if ((type & ResourceType::RenderTarget) != ResourceType::None)
        {
            _resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if ((type & ResourceType::DepthStencil) != ResourceType::None)
        {
            _resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        if ((type & ResourceType::DenyShader) != ResourceType::None)
        {
            _resourceDescription.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
    }
} // namespace dx12
