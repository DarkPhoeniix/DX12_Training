#include "DX12LibPCH.h"

#include "ResourceTable.h"

namespace dx12
{
    ResourceTable::ResourceTable(DescriptorHeapDescription descriptorHeapDesc, HeapDescription heapDesc)
        : _numDescriptors(descriptorHeapDesc.GetNumDescriptors())
    {
        _heap.Create(heapDesc);
        _heap.SetName("Heap of resource table");

        _descriptorHeap.Create(descriptorHeapDesc);
        _descriptorHeap.SetName("Descriptor heap of resource table");
    }

    bool ResourceTable::AddResource(Resource* resource, ResourceViewType viewType)
    {
        if (ASSERT(resource, "Trying to add a nullptr resource to resource table"))
        {
            return false;
        }

        ASSERT((_resources.size() < _numDescriptors), "Resource table is full");

        InternalResourceDesc value = { resource, _resources.size(), viewType };
        std::string key = resource->GetName();

        _resources.insert(std::make_pair(key, value));
        _descriptorHeap.PlaceResource(resource, viewType);
        _heap.PlaceResource(*resource);

        return true;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(Resource* resource, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceCPUHandle(resource, viewType);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(Resource* resource, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceGPUHandle(resource, viewType);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceCPUHandle(resourceName, viewType);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceGPUHandle(resourceName, viewType);
    }

    UINT ResourceTable::GetResourceIndex(Resource* resource, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceIndex(resource, viewType);
    }

    UINT ResourceTable::GetResourceIndex(const std::string& resourceName, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceIndex(resourceName, viewType);
    }

    Resource* ResourceTable::GetResourceByName(const std::string& resourceName, ResourceViewType viewType)
    {
        return _descriptorHeap.GetResourceByName(resourceName, viewType);
    }

    DescriptorHeap& ResourceTable::GetDescriptorHeap()
    {
        return _descriptorHeap;
    }

    const DescriptorHeap& ResourceTable::GetDescriptorHeap() const
    {
        return _descriptorHeap;
    }
} // namespace dx12
