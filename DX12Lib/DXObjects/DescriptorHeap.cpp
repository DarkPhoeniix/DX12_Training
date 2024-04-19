#include "stdafx.h"

#include "DescriptorHeap.h"

namespace Core
{
    DescriptorHeap::DescriptorHeap()
        : _descriptorHeap(nullptr)
        , _descriptorHeapDescription{}
        , _heapIncrementSize(0)
        , _DXDevice(Core::Device::GetDXDevice())
    {   }

    DescriptorHeap::DescriptorHeap(const DescriptorHeapDescription& description)
        : _descriptorHeap(nullptr)
        , _descriptorHeapDescription(description)
        , _heapIncrementSize(0)
        , _DXDevice(Core::Device::GetDXDevice())
    {   }

    DescriptorHeap::~DescriptorHeap()
    {
        _descriptorHeap = nullptr;
        _DXDevice = nullptr;
    }

    void DescriptorHeap::Create(const std::string& name)
    {
        if (!_DXDevice)
        {
            Logger::Log(LogType::Error, "Device is nullptr when trying to create descriptor heap");
        }

        _DXDevice->CreateDescriptorHeap(&_descriptorHeapDescription.GetDXDescription(), IID_PPV_ARGS(&_descriptorHeap));

        _name = name;
        std::wstring tmp(_name.begin(), _name.end());
        _descriptorHeap->SetName(tmp.c_str());

        _heapIncrementSize = _DXDevice->GetDescriptorHandleIncrementSize(_descriptorHeapDescription.GetType());

        for (UINT i = 0; i < _descriptorHeapDescription.GetNumDescriptors(); ++i)
        {
            _resourceIndex[i] = nullptr;
        }
    }

    void DescriptorHeap::PlaceResource(Resource* resource)
    {
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

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(Resource* resource)
    {
        if (!_DXDevice)
        {
            Logger::Log(LogType::Error, "Device is nullptr when trying to Get CPU descriptor handle increment size");
        }

        if (!resource)
        {
            Logger::Log(LogType::Error, "Trying to Get CPU handle for nullptr resource");
        }

        auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
        UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

        if (index == (UINT)-1)
        {
            Logger::Log(LogType::Error, "Trying to Get invalid resource CPU handle from descriptor heap");
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(Resource* resource)
    {
        if (!_DXDevice)
            Logger::Log(LogType::Error, "Device is nullptr when trying to Get GPU descriptor handle increment size");

        if (!resource)
            Logger::Log(LogType::Error, "Trying to Get GPU handle for nullptr resource");

        auto result = std::find_if(_resourceIndex.begin(), _resourceIndex.end(), [resource](const auto& pair) { return pair.second == resource; });
        UINT index = (result != _resourceIndex.end()) ? result->first : (UINT)-1;

        if (index == (UINT)-1)
            Logger::Log(LogType::Error, "Trying to Get invalid resource GPU handle from descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    void DescriptorHeap::SetDescription(const DescriptorHeapDescription& description)
    {
        _descriptorHeapDescription = description;
    }

    const DescriptorHeapDescription& DescriptorHeap::GetDescription() const
    {
        return _descriptorHeapDescription;
    }

    ComPtr<ID3D12DescriptorHeap> DescriptorHeap::GetDXDescriptorHeap() const
    {
        return _descriptorHeap;
    }
} // namespace Core
