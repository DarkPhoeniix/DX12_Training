#pragma once

#include "RHI/CommandList.h"
#include "RHI/DescriptorHeap.h"

using HeapIndex = std::uint32_t;
constexpr HeapIndex InvalidHeapIndex = HeapIndex(-1);

// A handle into the bindless descriptor heap, containing both CPU and GPU views.
struct DescriptorHandle
{
    HeapIndex Index = InvalidHeapIndex;
    rhi::CPUDescriptor CpuHandle = {};
    rhi::GPUDescriptor GpuHandle = {};
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
    ~DescriptorHeapManager() = default;

    static void Create(rhi::Device* device, std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors);
    static void Destroy();

    [[nodiscard]] static DescriptorHeapManager& Get();

    [[nodiscard]] DescriptorHandle AllocateStatic(DescriptorHeapType type);
    [[nodiscard]] DescriptorHandle AllocateTransient(DescriptorHeapType type);

    void ResetTransient();
    void Reset();

    void AdvanceFrameIndex();

    rhi::DescriptorHeap* GetShaderResourcesDescriptorHeap() const;

private:
    DescriptorHeapManager(rhi::Device* device, std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors);

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
    std::unique_ptr<rhi::DescriptorHeap> _RTVDescriptorHeap;
    std::unique_ptr<rhi::DescriptorHeap> _DSVDescriptorHeap;
    std::unique_ptr<rhi::DescriptorHeap> _shaderResourcesDescriptorHeap;

    DescriptorAllocator _RTVAllocator;
    DescriptorAllocator _DSVAllocator;
    DescriptorAllocator _staticAllocator;
    std::vector<DescriptorAllocator> _dynamicAllocator;

    rhi::Device* _device;

    static std::unique_ptr<DescriptorHeapManager> _instance;
};
