#pragma once

#include "Descriptor.h"

namespace rhi
{
    // DescriptorHeapType represents the type of descriptor heap, which determines the types of descriptors that can be stored in the heap and their intended usage
    enum class DescriptorHeapType
    {
        RTV,
        DSV,
        CBV_SRV_UAV
    };

    // DescriptorHeapDescription encapsulates the properties and configuration of a descriptor heap
    struct DescriptorHeapDescription
    {
        DescriptorHeapType Type = DescriptorHeapType::CBV_SRV_UAV;
        std::uint32_t NumDescriptors = 0;
        bool ShaderVisible = false;
        std::uint32_t Flags = 0;
    };

    // DescriptorHeap is an abstract interface representing a descriptor heap, which is a collection of descriptors
    class DescriptorHeap
    {
    public:
        DescriptorHeap() = default;
        DescriptorHeap(const DescriptorHeap&) = delete;
        DescriptorHeap(DescriptorHeap&&) noexcept = default;
        virtual ~DescriptorHeap() = default;

        DescriptorHeap& operator=(const DescriptorHeap&) = delete;
        DescriptorHeap& operator=(DescriptorHeap&&) noexcept = default;

        // Resets the descriptor heap, deleting all descriptors and allowing the heap to be reused for new descriptor allocations
        virtual void Reset() = 0;

        // Copies a resource descriptor into the descriptor heap and returns the offset of the copied descriptor within the heap.
        virtual std::uint32_t CopyResourceDescriptor(CPUDescriptor descriptor) = 0;

        // Retrieves the CPU handle for the start of the descriptor heap, allowing the application to access the base addresses for CPU descriptor access
        virtual CPUDescriptor GetHeapStartCPUHandle() = 0;
        // Retrieves the GPU handle for the start of the descriptor heap, allowing the application to access the base addresses for GPU descriptor access
        virtual GPUDescriptor GetHeapStartGPUHandle() = 0;

        // Retrieves the CPU handle for a descriptor at a specific offset within the descriptor heap, allowing the application to access descriptors at specific locations in the heap
        virtual CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) = 0;
        // Retrieves the GPU handle for a descriptor at a specific offset within the descriptor heap, allowing the application to access descriptors at specific locations in the heap
        virtual GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) = 0;

        // Increments the current offset in the descriptor heap by one, allowing the application to move the current position for descriptor allocations within the heap
        virtual std::uint32_t Offset() = 0;
        // Retrieves the current offset in the descriptor heap, which indicates the next available slot for descriptor allocation
        virtual std::uint32_t GetCurrentOffset() const = 0;

        // Retrieves the native descriptor heap object, allowing the application to access the underlying API-specific descriptor heap
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
