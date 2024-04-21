#pragma once

#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/Heap.h"

namespace Core
{
    class ResourceTable
    {
    public:
        ResourceTable(DescriptorHeapDescription descriptorHeapDesc, HeapDescription heapDesc);
        ~ResourceTable() = default;

        bool AddResource(Resource* resource);

        D3D12_CPU_DESCRIPTOR_HANDLE GetResourceCPUHandle(const std::string& name);
        D3D12_GPU_DESCRIPTOR_HANDLE GetResourceGPUHandle(const std::string& name);
        UINT GetResourceIndex(const std::string& name);

        DescriptorHeap& GetDescriptorHeap();
        const DescriptorHeap& GetDescriptorHeap() const;

    private:
        std::map<std::string, Resource*> _resources;
        DescriptorHeap _descriptorHeap;
        Heap _heap;
        int _numDescriptors;
    };
} // namespace Core
