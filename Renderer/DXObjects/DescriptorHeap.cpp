#include "stdafx.h"

#include "DescriptorHeap.h"

namespace Core
{
    DescriptorHeap::DescriptorHeap()
        : _descriptorHeap(nullptr)
        , _descriptorHeapDescription{}
        , _heapIncrementSize(0)
    {   }

    DescriptorHeap::DescriptorHeap(const DescriptorHeapDescription& description)
        : _descriptorHeap(nullptr)
        , _descriptorHeapDescription(description)
        , _heapIncrementSize(0)
    {   }

    DescriptorHeap::~DescriptorHeap()
    {
        _descriptorHeap = nullptr;
    }

    void DescriptorHeap::Create()
    {
        ASSERT(Core::Device::GetDXDevice(), "Device is nullptr when trying to create descriptor heap");

        Core::Device::GetDXDevice()->CreateDescriptorHeap(&_descriptorHeapDescription.GetDXDescription(), IID_PPV_ARGS(&_descriptorHeap));

        std::wstring tmp(_name.begin(), _name.end());
        _descriptorHeap->SetName(tmp.c_str());

        _heapIncrementSize = Core::Device::GetDXDevice()->GetDescriptorHandleIncrementSize(_descriptorHeapDescription.GetType());

        for (UINT i = 0; i < _descriptorHeapDescription.GetNumDescriptors(); ++i)
        {
            _resourceIndex[i] = nullptr;
        }
    }

    void DescriptorHeap::PlaceResource(Resource* resource)
    {
        // TODO: use vector instead of map
        for (auto& [index, res] : _resourceIndex)
        {
            if (!res)
            {
                _resourceIndex[index] = resource;
                return;
            }
        }

        Logger::Log(LogType::Error, "Descriptor heap " + _name + " doesn't have free desriptors");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartCPUHandle()
    {
        return _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartGPUHandle()
    {
        return _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapGPUHandle(size_t offset)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += offset * _heapIncrementSize;
        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(Resource* resource)
    {
        ASSERT(Core::Device::GetDXDevice(), "Device is nullptr when trying to Get CPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get CPU handle for nullptr resource");

        auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
        UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

        ASSERT((index != (UINT)-1), "Trying to Get invalid resource CPU handle from descriptor heap");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(Resource* resource)
    {
        ASSERT(Core::Device::GetDXDevice(), "Device is nullptr when trying to Get GPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get GPU handle for nullptr resource");

        auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
        UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

        ASSERT((index != (UINT)-1), "Trying to Get invalid resource GPU handle from descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    UINT DescriptorHeap::GetResourceIndex(Resource* resource)
    {
        auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
        ASSERT(result != _resourceIndex.end(), "Trying to Get invalid resource Index from descriptor heap");
        return result->first;
    }

    void DescriptorHeap::SetDescription(const DescriptorHeapDescription& description)
    {
        _descriptorHeapDescription = description;
    }

    const DescriptorHeapDescription& DescriptorHeap::GetDescription() const
    {
        return _descriptorHeapDescription;
    }

    void DescriptorHeap::SetName(const std::string& name)
    {
        _name = name;
        if (_descriptorHeap)
        {
            std::wstring tmp(_name.cbegin(), _name.cend());
            _descriptorHeap->SetName(tmp.c_str());
        }
    }

    const std::string& DescriptorHeap::GetName() const
    {
        return _name;
    }

    ComPtr<ID3D12DescriptorHeap> DescriptorHeap::GetDXDescriptorHeap() const
    {
        return _descriptorHeap;
    }
} // namespace Core
