#pragma once

namespace Core
{
    class HeapDescription
    {
    public:
        HeapDescription();
        ~HeapDescription() = default;

        void SetAlignment(UINT64 alignment);
        UINT64 GetAlignment() const;

        void SetHeapFlags(D3D12_HEAP_FLAGS heapFlags);
        D3D12_HEAP_FLAGS GetHeapFlags() const;

        void SetSize(UINT64 size);
        UINT64 GetSize() const;

        void SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY property);
        D3D12_CPU_PAGE_PROPERTY GetCPUPageProperty() const;

        void SetMemoryPoolPreference(D3D12_MEMORY_POOL memPoolPreference);
        D3D12_MEMORY_POOL GetMemoryPoolPreference() const;

        void SetCreationNodeMask(UINT nodeMask);
        UINT GetCreationNodeMask() const;

        void SetVisibleNodeMask(UINT nodeMask);
        UINT GetVisibleNodeMask() const;

        D3D12_HEAP_TYPE GetHeapType() const;
        void SetHeapType(D3D12_HEAP_TYPE heapType);

        const D3D12_HEAP_DESC& GetDXHeapDescription();

    private:
        D3D12_HEAP_DESC _heapDescription;
    };
} // namespace Core
