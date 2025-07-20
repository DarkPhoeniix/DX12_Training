#pragma once

#include "CommandList.h"
#include "ResourceTable.h"

// A handle into the bindless descriptor heap, containing both CPU and GPU views.
struct DescriptorHandle
{
    uint32_t Index;
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
};

class DescriptorHeapManager 
{
public:
    DescriptorHeapManager(std::uint64_t maxRTVDescriptors, std::uint64_t maxDSVDescriptors, std::uint64_t maxStaticDescriptors, std::uint64_t maxDynamicDescriptors);
    ~DescriptorHeapManager() = default;

    DescriptorHandle AllocateStatic();
    DescriptorHandle AllocateTransient();

    void ResetTransient();

    void Bind(dx12::CommandList& commandList);

    void AdvanceFrameIndex();

private:
    struct DescriptorAllocator
    {
        DescriptorAllocator() = default;
        DescriptorAllocator(std::uint64_t offset, std::uint64_t maxDescriptors);

        std::uint64_t Allocate();
        void Reset();

        std::uint64_t Offset;
        std::uint64_t MaxDescriptors;
        std::vector<bool> UsedDescriptors;
    };

    std::uint32_t _frameIndex;

    // Descriptor heaps for different types of resources: RTV, DSV, and CBV/SRV/UAV.
    dx12::DescriptorHeap _RTVDescriptorHeap;
    dx12::DescriptorHeap _DSVDescriptorHeap;
    dx12::DescriptorHeap _shaderResourcesDescriptorHeap;

    DescriptorAllocator _RTVAllocator;
    DescriptorAllocator _DSVAllocator;
    DescriptorAllocator _staticAllocator;
    std::vector<DescriptorAllocator> _dynamicAllocator;
};
