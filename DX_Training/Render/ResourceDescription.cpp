#include "stdafx.h"
#include "ResourceDescription.h"

ResourceDescription::ResourceDescription()
{
	_resourceDescription = D3D12_RESOURCE_DESC();

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

ResourceDescription::ResourceDescription(D3D12_RESOURCE_DESC desc)
{
	_resourceDescription = desc;
}

D3D12_RESOURCE_DESC ResourceDescription::CreateDXResourceDescription() const
{
    return _resourceDescription;
}

void ResourceDescription::SetDimension(D3D12_RESOURCE_DIMENSION dimension)
{
    _resourceDescription.Dimension = dimension;
}

D3D12_RESOURCE_DIMENSION ResourceDescription::GetDimension() const
{
    return _resourceDescription.Dimension;
}

void ResourceDescription::SetAlignment(UINT64 alignment)
{
    _resourceDescription.Alignment = alignment;
}

UINT64 ResourceDescription::GetAlignment() const
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
    return { (uint32_t)_resourceDescription.Width, (uint32_t)_resourceDescription.Height };
}

void ResourceDescription::SetDepthOrArraySize(UINT16 depthOrArraySize)
{
    _resourceDescription.DepthOrArraySize = depthOrArraySize;
}

UINT16 ResourceDescription::getDepthOrArraySize() const
{
    return _resourceDescription.DepthOrArraySize;
}

void ResourceDescription::SetMipLevels(UINT16 mipLevels)
{
    _resourceDescription.MipLevels = mipLevels;
}

UINT16 ResourceDescription::GetMipLevels() const
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

void ResourceDescription::AddFlags(D3D12_RESOURCE_FLAGS flags)
{
	_resourceDescription.Flags |= flags;
}

void ResourceDescription::SetFlags(D3D12_RESOURCE_FLAGS flags)
{
    _resourceDescription.Flags = flags;
}

D3D12_RESOURCE_FLAGS ResourceDescription::GetFlags() const
{
    return _resourceDescription.Flags;
}

void ResourceDescription::SetResourceType(EResourceType type)
{
    _resourceType = type;

	if (type & EResourceType::Texture)
	{
		_resourceDescription.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		_resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		_resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	}

	UpdateFlags(_resourceType);
	UpdateSize(_resourceType);
}

void ResourceDescription::AddResourceType(EResourceType type)
{
    _resourceType |= type;
}

EResourceType ResourceDescription::GetResourceType() const
{
    return _resourceType;
}

bool ResourceDescription::IsType(EResourceType type) const
{
    return bool(_resourceType & type);
}

void ResourceDescription::SetStride(UINT64 stride)
{
	_stride = stride;
}

UINT64 ResourceDescription::GetStride() const
{
	return _stride;
}

void ResourceDescription::SetClearValue(D3D12_CLEAR_VALUE clearValue)
{
	_clearValue = std::make_shared<D3D12_CLEAR_VALUE>(clearValue);
}

void ResourceDescription::SetClearValue(const DirectX::XMFLOAT4& clearValue)
{
	// TODO: clear here
	_clearValue->Color[0] = clearValue.x;
	_clearValue->Color[1] = clearValue.y;
	_clearValue->Color[2] = clearValue.z;
	_clearValue->Color[3] = clearValue.w;
}

std::shared_ptr<D3D12_CLEAR_VALUE> ResourceDescription::GetClearValue() const
{
	return _clearValue;
}

void ResourceDescription::UpdateSize(EResourceType type)
{
	if (type & EResourceType::Buffer)
	{
		unsigned int rowBytes = _resourceDescription.Width;

		if (type & EResourceType::StrideAlignment)
		{
			// calculate width of buffer
			unsigned int alligned = _stride;
			if (type & EResourceType::Dynamic)
			{
				alligned = (_stride + 255) & ~255;
			}
			rowBytes *= alligned;
		}
		else
		{
			// calculate width of buffer
			rowBytes *= _stride;
		}

        _resourceDescription.Width = rowBytes;
	}
}

void ResourceDescription::UpdateFlags(EResourceType type)
{
	if (type & EResourceType::Unordered)
	{
		_resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	if (type & EResourceType::RenderTarget)
	{
		_resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (type & EResourceType::DepthTarget)
	{
		_resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if (type & EResourceType::Deny_shader_resource)
	{
		_resourceDescription.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
}
