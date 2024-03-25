
#include "stdafx.h"

#include "Helpers.h"

#include <filesystem>

namespace Helper
{
    std::string HrToString(HRESULT hr)
    {
        char s_str[64] = {};
        sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(s_str);
    }

    void throwIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            Logger::Log(LogType::Error, "Raised an exception with code " + hr);
            throw HrException(hr);
        }
    }

    Json::Value ParseJson(const std::string& filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            Logger::Log(LogType::Error, "Failed to load " + filepath);

            return {};
        }

        std::ifstream file(filepath, std::ios_base::binary);
		file.open(filepath, std::ios_base::binary);
        Json::Value root;

        file >> root;

        return root;
    }

    ID3D12Resource* CreateBuffers(ComPtr<ID3D12Device2> device, EResourceType type, unsigned int size, unsigned int stride)
    {
        ID3D12Resource* resource = nullptr;
        D3D12_RESOURCE_DESC resourceDesc = {};

		{
			memset(&resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));

			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			if (type & EResourceType::Buffer)
			{
				unsigned int rowBytes = size;

				if (type & EResourceType::StrideAlignment)
				{
					// calculate width of buffer
					unsigned int alligned = stride;
					if (type & EResourceType::Dynamic)
					{
						alligned = (stride + 255) & ~255;
					}
					rowBytes *= alligned;
				}
				else
				{
					// calculate width of buffer
					rowBytes *= stride;
				}

				resourceDesc.Width = rowBytes;
				resourceDesc.Height = 1;
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}

			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			
			if (type & EResourceType::Unordered)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			if (type & EResourceType::RenderTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			if (type & EResourceType::DepthTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			if (type & EResourceType::Deny_shader_resource)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}
		}
		



		D3D12_RESOURCE_STATES startState = D3D12_RESOURCE_STATE_COPY_DEST;
		D3D12_HEAP_PROPERTIES heapDesc = {};
		{
			memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

			heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapDesc.CreationNodeMask = 1;
			heapDesc.VisibleNodeMask = 1;

			if (type & EResourceType::Dynamic)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				startState = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (type & EResourceType::ReadBack)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
			}
			else
			{
				heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
			}
		}

		// need to RTT and DTV
		const D3D12_CLEAR_VALUE* clearValue = nullptr;
		device->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			startState,
			clearValue, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&resource));


        return resource;
    }

	ID3D12Resource* CreateBuffers(ComPtr<ID3D12Device2> device, D3D12_RESOURCE_DESC resourceDesc, D3D12_CLEAR_VALUE* clearValue, D3D12_RESOURCE_STATES state)
	{
		ID3D12Resource* resource = nullptr;

		D3D12_RESOURCE_STATES startState = state;
		D3D12_HEAP_PROPERTIES heapDesc = {};
		{
			memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

			heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapDesc.CreationNodeMask = 1;
			heapDesc.VisibleNodeMask = 1;
			heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		}

		// need to RTT and DTV
		//const D3D12_CLEAR_VALUE* clearValue = nullptr;
		device->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			startState,
			clearValue, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&resource));

		return resource;
	}

	ID3D12Resource* CreateBuffers(ComPtr<ID3D12Device2> device, EResourceType type, unsigned int sizeW, unsigned int sizeH, unsigned int stride, D3D12_RESOURCE_STATES state)
	{
		ID3D12Resource* resource = nullptr;
		D3D12_RESOURCE_DESC resourceDesc = {};

		{
			memset(&resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));

			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			if (type & EResourceType::Buffer)
			{
				unsigned int rowBytes = sizeW;

				if (type & EResourceType::StrideAlignment)
				{
					// calculate width of buffer
					unsigned int alligned = stride;
					if (type & EResourceType::Dynamic)
					{
						alligned = (stride + 255) & ~255;
					}
					rowBytes *= alligned;
				}
				else
				{
					// calculate width of buffer
					rowBytes *= stride;
				}

				resourceDesc.Width = rowBytes;
				resourceDesc.Height = sizeH;
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}

			if (type & EResourceType::Texture)
			{
				resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
				resourceDesc.Width = sizeW;
				resourceDesc.Height = sizeH;
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			}

			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			if (type & EResourceType::Unordered)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			if (type & EResourceType::RenderTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			if (type & EResourceType::DepthTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			if (type & EResourceType::Deny_shader_resource)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}
		}




		D3D12_RESOURCE_STATES startState = state;
		D3D12_HEAP_PROPERTIES heapDesc = {};
		{
			memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

			heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapDesc.CreationNodeMask = 1;
			heapDesc.VisibleNodeMask = 1;

			if (type & EResourceType::Dynamic)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				startState = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (type & EResourceType::ReadBack)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
			}
			else
			{
				heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
			}
		}

		// need to RTT and DTV
		const D3D12_CLEAR_VALUE* clearValue = nullptr;
		device->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			startState,
			clearValue, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&resource));


		return resource;
	}

	ID3D12Resource* CreateBuffers(ID3D12Heap* pHeap, ComPtr<ID3D12Device2> device, EResourceType type, unsigned int size, unsigned int stride, unsigned int offset)
	{
		ID3D12Resource* resource = nullptr;
		D3D12_RESOURCE_DESC resourceDesc = {};

		{
			memset(&resourceDesc, 0, sizeof(D3D12_RESOURCE_DESC));

			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			if (type & EResourceType::Buffer)
			{
				unsigned int rowBytes = size;

				if (type & EResourceType::StrideAlignment)
				{
					// calculate width of buffer
					unsigned int alligned = stride;
					if (type & EResourceType::Dynamic)
					{
						alligned = (stride + 255) & ~255;
					}
					rowBytes *= alligned;
				}
				else
				{
					// calculate width of buffer
					rowBytes *= stride;
				}

				resourceDesc.Width = rowBytes;
				resourceDesc.Height = 1;
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			}

			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			if (type & EResourceType::Unordered)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
			if (type & EResourceType::RenderTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}
			if (type & EResourceType::DepthTarget)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			}
			if (type & EResourceType::Deny_shader_resource)
			{
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}
		}




		D3D12_RESOURCE_STATES startState = D3D12_RESOURCE_STATE_COPY_DEST;
		D3D12_HEAP_PROPERTIES heapDesc = {};
		{
			memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

			heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapDesc.CreationNodeMask = 1;
			heapDesc.VisibleNodeMask = 1;

			if (type & EResourceType::Dynamic)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				startState = D3D12_RESOURCE_STATE_GENERIC_READ;
			}
			else if (type & EResourceType::ReadBack)
			{
				heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
			}
			else
			{
				heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
			}
		}

		// need to RTT and DTV
		const D3D12_CLEAR_VALUE* clearValue = nullptr;
		device->CreatePlacedResource(
			pHeap,
			offset,
			&resourceDesc,
			startState,
			clearValue, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
			IID_PPV_ARGS(&resource));


		return resource;
	}
}
