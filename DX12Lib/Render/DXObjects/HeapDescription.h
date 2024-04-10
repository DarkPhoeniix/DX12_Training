#pragma once

class HeapDescription
{
public:
    HeapDescription();
    ~HeapDescription() = default;

    UINT64 GetAlignment() const;
    void SetAlignment(UINT64 alignment);

    D3D12_HEAP_FLAGS GetHeapFlags() const;
    void SetHeapFlags(D3D12_HEAP_FLAGS heapFlags);

    UINT64 GetSize() const;
    void SetSize(UINT64 size);

    D3D12_CPU_PAGE_PROPERTY GetCPUPageProperty() const;
    void SetCPUPageProperty(D3D12_CPU_PAGE_PROPERTY property);

    D3D12_MEMORY_POOL GetMemoryPoolPreference() const;
    void SetMemoryPoolPreference(D3D12_MEMORY_POOL memPoolPreference);

    UINT GetCreationNodeMask() const;
    void SetCreationNodeMask(UINT nodeMask);

    UINT GetVisibleNodeMask() const;
    void SetVisibleNodeMask(UINT nodeMask);

    D3D12_HEAP_TYPE GetHeapType() const;
    void SetHeapType(D3D12_HEAP_TYPE heapType);

    const D3D12_HEAP_DESC& GetDXHeapDescription();

private:
    D3D12_HEAP_DESC _heapDescription;
};
