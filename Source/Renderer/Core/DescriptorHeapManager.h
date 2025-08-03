#pragma once

#include "CommandList.h"

using HeapIndex = std::uint32_t;
constexpr HeapIndex InvalidHeapIndex = HeapIndex(-1);

// A handle into the bindless descriptor heap, containing both CPU and GPU views.
struct DescriptorHandle
{
    HeapIndex Index;
    D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle;
};

enum class DescriptorHeapType
{
    RTV, // Render Target View
    DSV, // Depth Stencil View
    Static, // Static resources (CBV/SRV/UAV)
    Dynamic // Dynamic resources (CBV/SRV/UAV)
};

class DescriptorHeapManager 
{
public:
    DescriptorHeapManager(std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors);
    ~DescriptorHeapManager() = default;

    [[nodiscard]] DescriptorHandle AllocateStatic(DescriptorHeapType type);
    [[nodiscard]] DescriptorHandle AllocateTransient(DescriptorHeapType type);

    void ResetTransient();
    void Reset();

    void Bind(dx12::CommandList& commandList);

    void AdvanceFrameIndex();

    const dx12::DescriptorHeap& GetShaderResourcesDescriptorHeap() const;

private:
    struct DescriptorAllocator
    {
        DescriptorAllocator() = default;
        DescriptorAllocator(std::uint32_t start, std::uint32_t maxDescriptors);

        HeapIndex Allocate();
        void Reset();

        std::uint32_t Start;
        std::uint32_t Offset;
        std::uint32_t MaxDescriptors;
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
