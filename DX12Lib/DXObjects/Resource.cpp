#include "stdafx.h"

#include "Resource.h"

namespace Core
{
	Resource::Resource()
		: _DXDevice(Core::Device::GetDXDevice())
		, _resource(nullptr)
		, _resourceDesc{}
		, _currentState(D3D12_RESOURCE_STATE_COMMON)
		, _initialState(D3D12_RESOURCE_STATE_COMMON)
	{
	}

	Resource::Resource(ResourceDescription resourceDesc)
		: _DXDevice(Core::Device::GetDXDevice())
		, _resource(nullptr)
		, _resourceDesc(resourceDesc)
		, _currentState(D3D12_RESOURCE_STATE_COMMON)
		, _initialState(D3D12_RESOURCE_STATE_COMMON)
	{
	}

	Resource::~Resource()
	{
		_DXDevice = nullptr;
		_resource = nullptr;
	}

	void Resource::InitFromDXResource(ComPtr<ID3D12Resource> resource)
	{
		_resource = resource;
		_resourceDesc = resource->GetDesc();
		_initialState = D3D12_RESOURCE_STATE_COMMON;
		_currentState = D3D12_RESOURCE_STATE_COMMON;
	}

	ComPtr<ID3D12Resource> Resource::GetDXResource() const
	{
		return _resource;
	}

	ComPtr<ID3D12Resource>& Resource::GetDXResource()
	{
		return _resource;
	}

	void Resource::SetName(const std::string& name)
	{
		_name = name;
		if (_resource)
		{
			std::wstring tmp(_name.begin(), _name.end());
			_resource->SetName(tmp.c_str());
		}
	}

	std::string Resource::GetName() const
	{
		return _name;
	}

	void Resource::SetResourceDescription(const ResourceDescription& resourceDesc)
	{
		_resourceDesc = resourceDesc;
	}

	ResourceDescription Resource::GetResourceDescription() const
	{
		return _resourceDesc;
	}

	void Resource::SetCurrentState(D3D12_RESOURCE_STATES state)
	{
		_currentState = state;
	}

	D3D12_RESOURCE_STATES Resource::GetCurrentState() const
	{
		return _currentState;
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

	D3D12_RESOURCE_BARRIER Resource::CreateBarrierAlias(Resource* old) const
	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Aliasing.pResourceBefore = old ? old->GetDXResource().Get() : nullptr;
		barrier.Aliasing.pResourceAfter = _resource.Get();

		return barrier;
	}

	D3D12_GPU_VIRTUAL_ADDRESS Resource::OffsetGPU(unsigned int offset) const
	{
		ASSERT(_resource, "Trying to get GPU pointer for an nullptr resource");

		D3D12_GPU_VIRTUAL_ADDRESS result = _resource->GetGPUVirtualAddress();
		result += offset;

		return result;
	}

	ComPtr<ID3D12Resource> Resource::CreateCommitedResource(D3D12_RESOURCE_STATES initialState)
	{
		_initialState = initialState;
		_currentState = _initialState;

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
		D3D12_RESOURCE_DESC resourceDesc = _resourceDesc.CreateDXResourceDescription();
		D3D12_CLEAR_VALUE* clearValue = _resourceDesc.GetClearValue().get();
		_DXDevice->CreateCommittedResource(
			&heapDesc,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			_initialState,
			clearValue, // TODO: check
			IID_PPV_ARGS(&_resource));

		std::wstring temp(_name.begin(), _name.end());
		_resource->SetName(temp.c_str());

		return _resource;
	}

	ComPtr<ID3D12Resource> Resource::CreatePlacedResource(ComPtr<ID3D12Heap> heap, unsigned int offset, D3D12_RESOURCE_STATES initialState)
	{
		_initialState = initialState;

		D3D12_RESOURCE_DESC resourceDesc = _resourceDesc.CreateDXResourceDescription();
		D3D12_CLEAR_VALUE* clearValue = _resourceDesc.GetClearValue().get();
		_DXDevice->CreatePlacedResource(
			heap.Get(),
			offset,
			&resourceDesc,
			_initialState,
			clearValue, // TODO: check
			IID_PPV_ARGS(&_resource));

		std::wstring temp(_name.begin(), _name.end());
		_resource->SetName(temp.c_str());

		return _resource;
	}
} // namespace Core
