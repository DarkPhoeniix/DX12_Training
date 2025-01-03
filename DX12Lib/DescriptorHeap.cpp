#include "DX12LibPCH.h"

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
    }

    void DescriptorHeap::Create(const DescriptorHeapDescription& description)
    {
        _description = description;

        Create();
    }

    void DescriptorHeap::Reset()
    {
        _resources.clear();
    }

    void DescriptorHeap::PlaceResource(Resource* resource, ResourceViewType viewType)
    {
        InternalResourceDesc value = { _resources.size(), viewType };
        std::string key = resource->GetName();

        _resources.insert(std::make_pair(key, value));
    }

    void DescriptorHeap::CopyResourceDescriptor(Resource* resource, ResourceViewType viewType, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        InternalResourceDesc value = { _resources.size(), viewType };
        std::string key = resource->GetName();

        _resources.insert(std::make_pair(key, value));

        uint32_t offset = value.HeapIndex;
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

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceCPUHandle(Resource* resource, ResourceViewType viewType)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to Get CPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get CPU handle for nullptr resource");

        std::string key = resource->GetName();
        auto result = _resources.find(key);
        while (result->first == key && result->second.Type != viewType)
        {
            ASSERT((result != _resources.end()), "Trying to Get invalid resource CPU handle from descriptor heap");
            ++result;
        }
        
        ASSERT((result != _resources.end()), "Trying to Get invalid resource CPU handle from descriptor heap");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * result->second.HeapIndex;

        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetResourceGPUHandle(Resource* resource, ResourceViewType viewType)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to Get GPU descriptor handle increment size");
        ASSERT(resource, "Trying to Get GPU handle for nullptr resource");

        std::string key = resource->GetName();
        auto result = _resources.find(key);
        while (result->first == key && result->second.Type != viewType)
        {
            ++result;
        }

        ASSERT((result != _resources.end()), "Trying to Get invalid resource GPU handle from descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * result->second.HeapIndex;

        return handle;
    }

    UINT DescriptorHeap::GetResourceIndex(Resource* resource, ResourceViewType viewType)
    {
        std::string key = resource->GetName();
        auto result = _resources.find(key);
        while (result->first == key && result->second.Type != viewType)
        {
            ++result;
        }

        ASSERT((result != _resources.end()), "Trying to Get invalid resource GPU handle from descriptor heap");

        return result->second.HeapIndex;
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
