#include "stdafx.h"

#include "ResourceTable.h"

namespace Core
{
    ResourceTable::ResourceTable(DescriptorHeapDescription descriptorHeapDesc, HeapDescription heapDesc)
        : _numDescriptors(descriptorHeapDesc.GetNumDescriptors())
    {
        _heap.SetDescription(heapDesc);
        _heap.SetName("Heap of resource table");
        _heap.Create();

        _descriptorHeap.SetDescription(descriptorHeapDesc);
        _descriptorHeap.SetName("Descriptor heap of resource table");
        _descriptorHeap.Create();
    }

    bool ResourceTable::AddResource(Resource* resource)
    {
        bool res = false;

        ASSERT((_resources.size() < _numDescriptors), "Resource table is full");
        ASSERT(resource, "Trying to add a nullptr resource to resource table");

        auto name = resource->GetName();
        LOG_WARNING(!name.empty(), "Resource in resource table is unnamed");

        auto it = _resources.find(name);
        if (it == _resources.end())
        {
            _resources.emplace(name, resource);
            _descriptorHeap.PlaceResource(resource);
            _heap.PlaceResource(*resource);

            res = true;
        }

        return res;
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
} // namespace Core
