#include "pch.h"

#include "DescriptorHeap.h"

namespace dx12
{
    DescriptorHeap::DescriptorHeap()
        : _descriptorHeap(nullptr)
        , _description{}
        , _heapIncrementSize(0)
    {   }

    DescriptorHeap::DescriptorHeap(const DescriptorHeapDescription& description)
        : _descriptorHeap(nullptr)
        , _description(description)
        , _heapIncrementSize(0)
    {   }

    DescriptorHeap::~DescriptorHeap()
    {
        _descriptorHeap = nullptr;
    }

    void DescriptorHeap::Create()
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to create descriptor heap");

        dx12::Device::GetDXDevice()->CreateDescriptorHeap(&_description.GetDXDescription(), IID_PPV_ARGS(&_descriptorHeap));

        std::wstring tmp(_name.begin(), _name.end());
        _descriptorHeap->SetName(tmp.c_str());

        _heapIncrementSize = dx12::Device::GetDXDevice()->GetDescriptorHandleIncrementSize(_description.GetType());

        int numDescriptors = _description.GetNumDescriptors();
        _resources.resize(numDescriptors, nullptr);
    }

    void DescriptorHeap::Create(const DescriptorHeapDescription& description)
    {
        _description = description;

        Create();
    }

    void DescriptorHeap::Reset()
    {
        for (auto& res : _resources)
        {
            res = nullptr;
        }
    }

    uint32_t DescriptorHeap::PlaceResource(Resource* resource)
    {
        size_t offset = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (!_resources[i])
            {
                offset = i;
                _resources[i] = resource;
                break;
            }
        }

        return offset;
    }

    void DescriptorHeap::PlaceResourceDescriptor(Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        size_t offset = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (!_resources[i])
            {
                offset = i;
                _resources[i] = resource;
            }
        }

        if (ASSERT(offset != -1, std::format("Descriptor heap {} is full", _name)))
        {
            return;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * offset;
        dx12::Device::GetDXDevice()->CopyDescriptorsSimple(1, handle, descriptor, _description.GetType());
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartCPUHandle()
    {
        return _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartGPUHandle()
    {
        return _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetOffsetCPUHandle(uint32_t index)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetOffsetGPUHandle(uint32_t index)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetFreeCPUHandle()
    {
        size_t offset = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (!_resources[i])
            {
                offset = i;
                break;
            }
        }

        ASSERT((offset != -1), "Trying to Get invalid resource CPU handle from descriptor heap");

        D3D12_CPU_DESCRIPTOR_HANDLE start = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        start.ptr += _heapIncrementSize * offset;

        return start;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetFreeGPUHandle()
    {
        size_t offset = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (!_resources[i])
            {
                offset = i;
                break;
            }
        }

        ASSERT((offset != -1), "Trying to Get invalid resource CPU handle from descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE start = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        start.ptr += _heapIncrementSize * offset;

        return start;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(Resource* resource)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to Get CPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get CPU handle for nullptr resource");

        auto result = std::find_if(_resources.begin(), _resources.end(), [resource](const auto& res) { return res == resource; });
        
        size_t index = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (_resources[i] == resource)
            {
                index = i;
                break;
            }
        }

        ASSERT((index != -1), "Trying to Get invalid resource CPU handle from descriptor heap");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(Resource* resource)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to Get GPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get GPU handle for nullptr resource");

        size_t index = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (_resources[i] == resource)
            {
                index = i;
                break;
            }
        }

        ASSERT((index != (UINT)-1), "Trying to Get invalid resource GPU handle from descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * index;

        return handle;
    }

    UINT DescriptorHeap::GetResourceIndex(Resource* resource)
    {
        size_t index = -1;
        for (size_t i = 0; i < _resources.size(); ++i)
        {
            if (_resources[i] == resource)
            {
                index = i;
                break;
            }
        }

        ASSERT((index != (UINT)-1), "Trying to Get invalid resource GPU handle from descriptor heap");

        return index;
    }

    void DescriptorHeap::SetDescription(const DescriptorHeapDescription& description)
    {
        _description = description;
    }

    const DescriptorHeapDescription& DescriptorHeap::GetDescription() const
    {
        return _description;
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
} // namespace dx12
