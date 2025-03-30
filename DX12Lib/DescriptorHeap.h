#pragma once

#include "DescriptorHeapDescription.h"

namespace dx12
{
    // Enum defining different types of resource views that can be stored in descriptor heaps.
    enum class ResourceViewType
    {
        Unknown, // Default or uninitialized type.
        RTV,     // Render Target View.
        DSV,     // Depth Stencil View.
        CBV,     // Constant Buffer View.
        SRV,     // Shader Resource View.
        UAV      // Unordered Access View.
    };

    // Enum defining types of descriptor heaps available in DirectX 12.
    enum class DescriptorHeapType
    {
        RTV,
        DSV,
        CBV_SRV_UAV
    };

    // Class representing a wrapper for DirectX 12 descriptor heap, used for managing resource descriptors.
    class DescriptorHeap
    {
    public:
        // Default constructor, initializes an empty descriptor heap.
        DescriptorHeap();
        // Constructor to initialize a descriptor heap with a given description.
        DescriptorHeap(const DescriptorHeapDescription& description);
        DescriptorHeap(DescriptorHeap&&) = default;
        // Destructor to properly release the descriptor heap.
        ~DescriptorHeap();

        DescriptorHeap& operator=(DescriptorHeap&&) = default;

        // Creates a descriptor heap based on the stored description.
        void Create();
        // Creates a descriptor heap using a new description.
        void Create(const DescriptorHeapDescription& description);

        // Resets the descriptor heap, clearing all descriptors.
        void Reset();

        // Copies a resource descriptor into the heap and returns its offset.
        std::uint32_t CopyResourceDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        // Gets the starting CPU descriptor handle for this heap.
        D3D12_CPU_DESCRIPTOR_HANDLE GetHeapStartCPUHandle();
        // Gets the starting GPU descriptor handle for this heap.
        D3D12_GPU_DESCRIPTOR_HANDLE GetHeapStartGPUHandle();

        // Retrieves a CPU descriptor handle at a given offset from the heap start.
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleWithOffset(std::uint32_t offset);
        // Retrieves a GPU descriptor handle at a given offset from the heap start.
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleWithOffset(std::uint32_t offset);

        // Increments and returns the current descriptor offset in the heap.
        std::uint32_t Offset();
        // Returns the current offset within the descriptor heap.
        std::uint32_t GetCurrentOffset() const;

        // Sets the descriptor heap's description.
        void SetDescription(const DescriptorHeapDescription& description);
        // Gets the descriptor heap's description.
        const DescriptorHeapDescription& GetDescription() const;

        // Sets a name for the descriptor heap (useful for debugging).
        void SetName(const std::string& name);
        // Retrieves the name of the descriptor heap.
        const std::string& GetName() const;

        // Retrieves the DirectX 12 descriptor heap object.
        ComPtr<ID3D12DescriptorHeap> GetDXDescriptorHeap() const;

    private:
        // The DirectX 12 descriptor heap.
        ComPtr<ID3D12DescriptorHeap> _descriptorHeap;
        // The descriptor heap description containing heap properties.
        DescriptorHeapDescription _description;

        // The descriptor increment size, determining how far apart descriptors are spaced.
        UINT _heapIncrementSize;
        // The current offset in the heap for tracking descriptor allocations.
        std::uint32_t _currentOffset;

        // The name of the descriptor heap (for debugging).
        std::string _name;
    };
} // namespace dx12
