#pragma once

#include "DescriptorHeap.h"

namespace rhi::d3d12
{
    // Class representing a wrapper for DirectX 12 descriptor heap, used for managing resource descriptors.
    class D3D12DescriptorHeap final : public rhi::DescriptorHeap
    {
    public:
        // Copy constructor.
        D3D12DescriptorHeap(const D3D12DescriptorHeap& other) = delete;
        // Move constructor.
        D3D12DescriptorHeap(D3D12DescriptorHeap&& other) noexcept;
        // Destructor to properly release the descriptor heap.
        ~D3D12DescriptorHeap();

        // Copy assignment operator.
        D3D12DescriptorHeap& operator=(const D3D12DescriptorHeap& other) = delete;
        // Move assignment operator.
        D3D12DescriptorHeap& operator=(D3D12DescriptorHeap&& other) noexcept;

        // Resets the descriptor heap, clearing all descriptors.
        void Reset() override;

        // Copies a resource descriptor into the heap and returns its offset.
        std::uint32_t CopyResourceDescriptor(rhi::CPUDescriptor descriptor) override;

        // Gets the starting CPU descriptor handle for this heap.
        rhi::CPUDescriptor GetHeapStartCPUHandle() override;
        // Gets the starting GPU descriptor handle for this heap.
        rhi::GPUDescriptor GetHeapStartGPUHandle() override;

        // Retrieves a CPU descriptor handle at a given offset from the heap start.
        rhi::CPUDescriptor GetCPUHandleWithOffset(std::uint32_t offset) override;
        // Retrieves a GPU descriptor handle at a given offset from the heap start.
        rhi::GPUDescriptor GetGPUHandleWithOffset(std::uint32_t offset) override;

        // Increments and returns the current descriptor offset in the heap.
        std::uint32_t Offset() override;
        // Returns the current offset within the descriptor heap.
        std::uint32_t GetCurrentOffset() const override;

        // Retrieves the DirectX 12 descriptor heap object.
        void* GetNative() const override;

    private:
        friend class D3D12Device;

        // Default constructor, initializes an empty descriptor heap.
        D3D12DescriptorHeap(rhi::Device* device, const rhi::DescriptorHeapDescription& description, [[maybe_unused]] const std::string& name = "");

        // The DirectX 12 descriptor heap.
        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        // The descriptor heap description containing heap properties.
        rhi::DescriptorHeapDescription _description;

        // The descriptor increment size, determining how far apart descriptors are spaced.
        UINT _heapIncrementSize;
        // The current offset in the heap for tracking descriptor allocations.
        std::uint32_t _currentOffset;

        rhi::Device* _device;
#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
