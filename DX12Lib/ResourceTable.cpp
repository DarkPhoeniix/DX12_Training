#include "pch.h"

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

    bool ResourceTable::AddResource(Resource* resource)
    {
        if (ASSERT(resource, "Trying to add a nullptr resource to resource table"))
        {
            return false;
        }

        std::string resourceName = resource->GetName();

        ASSERT((_resources.size() < _numDescriptors), "Resource table is full");
        ASSERT(!resourceName.empty(), "Resource in resource table is unnamed");

        auto it = _resources.find(resourceName);
        if (it == _resources.end())
        {
            _resources.emplace(resourceName, resource);
            _descriptorHeap.PlaceResource(resource);
            _heap.PlaceResource(*resource);

            return true;
        }

        return false;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceCPUHandle(const std::string& name)
    {
        Resource* resource = _resources[name];
        ASSERT(resource, "Resource \"" + name + "\" not present in resource table");
        return _descriptorHeap.GetResourceCPUHandle(resource);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ResourceTable::GetResourceGPUHandle(const std::string& name)
    {
        Resource* resource = _resources[name];
        ASSERT(resource, "Resource \"" + name + "\" not present in resource table");
        return _descriptorHeap.GetResourceGPUHandle(resource);
    }

    UINT ResourceTable::GetResourceIndex(const std::string& name)
    {
        Resource* resource = _resources[name];
        ASSERT(resource, "Resource \"" + name + "\" not present in resource table");
        return _descriptorHeap.GetResourceIndex(resource);
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
