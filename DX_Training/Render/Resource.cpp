#include "stdafx.h"
#include "Resource.h"

Resource::Resource(ResourceDescription resourceDesc, ComPtr<ID3D12Device> device)
	: _resourceDesc(resourceDesc)
	, _device(device)
{	}

Resource::~Resource()
{
	_resource->Release();
}

void Resource::SetResource(ComPtr<ID3D12Resource> resource)
{
	_resource = resource;
}

ComPtr<ID3D12Resource> Resource::GetResource() const
{
	return _resource;
}

void Resource::SetResourceDescription(const ResourceDescription& resourceDesc)
{
	_resourceDesc = resourceDesc;
}

ResourceDescription Resource::GetResourceDescription() const
{
	return _resourceDesc;
}

void* Resource::Map()
{
	void* res = nullptr;

	D3D12_RANGE range;
	range.Begin = 0;
	range.End = 0;
	_resource->Map(0, &range, &res);

	return res;
}

D3D12_GPU_VIRTUAL_ADDRESS Resource::OffsetGPU(unsigned int offset) const
{
	//assert(!this->_resource && "impossible to get GPU pointer in empty resource");

	D3D12_GPU_VIRTUAL_ADDRESS result = this->_resource->GetGPUVirtualAddress();
	result += offset;

	return result;
}

ComPtr<ID3D12Resource> Resource::CreateCommitedResource()
{
	ComPtr<ID3D12Resource> resource = nullptr;

	D3D12_HEAP_PROPERTIES heapDesc = {};
	{
		memset(&heapDesc, 0, sizeof(D3D12_HEAP_PROPERTIES));

		heapDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.CreationNodeMask = 1;
		heapDesc.VisibleNodeMask = 1;

		if (_resourceDesc.IsType(EResourceType::Dynamic))
		{
			heapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
			_initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		else if (_resourceDesc.IsType(EResourceType::ReadBack))
		{
			heapDesc.Type = D3D12_HEAP_TYPE_READBACK;
		}
		else
		{
			heapDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
		}
	}

	// need to RTT and DSV
	const D3D12_CLEAR_VALUE* clearValue = nullptr;
	_device->CreateCommittedResource(
		&heapDesc,
		D3D12_HEAP_FLAG_NONE,
		&_resourceDesc.CreateDXResourceDescription(),
		_initialState,
		_resourceDesc.GetClearValue(), // TODO: check
		IID_PPV_ARGS(&resource));

	return resource;
}

ComPtr<ID3D12Resource> Resource::CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset)
{
	ComPtr<ID3D12Resource> resource = nullptr;

	const D3D12_CLEAR_VALUE* clearValue = nullptr;
	_device->CreatePlacedResource(
		heap.Get(),
		offset,
		&_resourceDesc.CreateDXResourceDescription(),
		_initialState,
		_resourceDesc.GetClearValue(), // TODO: check
		IID_PPV_ARGS(&resource));

	return resource;
}
