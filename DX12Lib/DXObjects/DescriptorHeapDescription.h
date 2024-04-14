#pragma once

class DescriptorHeapDescription
{
public:
    DescriptorHeapDescription();
    ~DescriptorHeapDescription() = default;

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDXDescription() const;

    void SetType(D3D12_DESCRIPTOR_HEAP_TYPE type);
    D3D12_DESCRIPTOR_HEAP_TYPE GetType() const;

    void SetNumDescriptors(UINT num);
    UINT GetNumDescriptors() const;

    void SetFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flags);
    D3D12_DESCRIPTOR_HEAP_FLAGS GetFlags() const;

    void SetNodeMask(UINT mask);
    UINT GetNodeMask() const;

private:
    D3D12_DESCRIPTOR_HEAP_DESC _description;
};
