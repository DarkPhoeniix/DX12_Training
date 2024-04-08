#pragma once

class DescriptorHeapDescription
{
public:
    DescriptorHeapDescription();
    ~DescriptorHeapDescription() = default;

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDXDescription() const;

    D3D12_DESCRIPTOR_HEAP_TYPE GetType() const;
    void SetType(D3D12_DESCRIPTOR_HEAP_TYPE type);

    UINT GetNumDescriptors() const;
    void SetNumDescriptors(UINT num);

    D3D12_DESCRIPTOR_HEAP_FLAGS GetFlags() const;
    void SetFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flags);

    UINT GetNodeMask() const;
    void SetNodeMask(UINT mask);

private:
    D3D12_DESCRIPTOR_HEAP_DESC _description;
};
