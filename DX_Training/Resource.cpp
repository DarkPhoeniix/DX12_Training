#include "stdafx.h"
#include "Resource.h"

Resource::Resource(ID3D12Resource* resource)
{
	_resource = resource;
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
