#pragma once

namespace dx12
{
    // Wrapper for D3D12_HEAP_DESC to configure a heap (memory block).
    class HeapDescription
    {
    public:
        HeapDescription() = default;
        HeapDescription(const HeapDescription&) = default;
        HeapDescription(HeapDescription&&) = default;

        HeapDescription& operator=(const HeapDescription&) = default;
        HeapDescription& operator=(HeapDescription&&) = default;

        // Set the heap type.
        void SetHeapType(D3D12_HEAP_TYPE heapType);
        // Get the heap type.
        D3D12_HEAP_TYPE GetHeapType() const;

        // Set the heap size (before alignment adjustments).
        void SetSize(UINT64 size);
        // Get the heap size (before alignment adjustments).
        UINT64 GetSize() const;

        // Set the heap alignment.
        void SetAlignment(UINT64 alignment);
        // Get the heap alignment.
        UINT64 GetAlignment() const;

        // Set the heap flags.
        void SetHeapFlags(D3D12_HEAP_FLAGS heapFlags);
        // Get the heap flags.
        D3D12_HEAP_FLAGS GetHeapFlags() const;

        // Set the CPU page property of the heap.
        void SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY property);
        // Get the CPU page property of the heap.
        D3D12_CPU_PAGE_PROPERTY GetCPUPageProperty() const;

        // Set the memory pool preference (e.g., default, L0, L1).
        void SetMemoryPoolPreference(D3D12_MEMORY_POOL memPoolPreference);
        // Get the memory pool preference.
        D3D12_MEMORY_POOL GetMemoryPoolPreference() const;

        // Set the heap creation node mask.
        void SetCreationNodeMask(UINT nodeMask);
        // Get the heap creation node mask.
        UINT GetCreationNodeMask() const;

        // Set the heap visible node mask.
        void SetVisibleNodeMask(UINT nodeMask);
        // Get the heap visible node mask.
        UINT GetVisibleNodeMask() const;

        // Get the raw D3D12 heap description.
        const D3D12_HEAP_DESC& GetDXHeapDescription();

    private:
        // Raw D3D12 heap description.
        D3D12_HEAP_DESC _heapDescription = {};
    };
} // namespace dx12
