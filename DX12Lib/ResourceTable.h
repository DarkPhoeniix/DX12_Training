#pragma once

#include "DescriptorHeap.h"
#include "Heap.h"

namespace dx12
{
    class ResourceTable
    {
    public:
        ResourceTable(DescriptorHeapDescription descriptorHeapDesc, HeapDescription heapDesc);
        ~ResourceTable() = default;

        bool AddResource(Resource* resource, ResourceViewType viewType);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(Resource* resource, ResourceViewType viewType);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(Resource* resource, ResourceViewType viewType);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(const std::string& resourceName, ResourceViewType viewType);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(const std::string& resourceName, ResourceViewType viewType);

        UINT GetResourceIndex(Resource* resource, ResourceViewType viewType);
        UINT GetResourceIndex(const std::string& resourceName, ResourceViewType viewType);

        Resource* GetResourceByName(const std::string& resourceName, ResourceViewType viewType);

        DescriptorHeap& GetDescriptorHeap();
        const DescriptorHeap& GetDescriptorHeap() const;

    private:
        struct InternalResourceDesc
        {
            using ResourceIndex = std::uint32_t;

            Resource* Res;
            ResourceIndex HeapIndex = -1;
            ResourceViewType Type = ResourceViewType::Unknown;
        };

        std::unordered_multimap<std::string, InternalResourceDesc> _resources;

        DescriptorHeap _descriptorHeap;
        Heap _heap;
        int _numDescriptors;
    };
} // namespace dx12
