#pragma once

namespace dx12
{
    // Wrapper for D3D12_DESCRIPTOR_HEAP_DESC to configure a descriptor heap.
    class DescriptorHeapDescription
    {
    public:
        // Default initializaion
        DescriptorHeapDescription();
        // Copy constructor.
        DescriptorHeapDescription(const DescriptorHeapDescription& other) = default;
        // Destructor.
        ~DescriptorHeapDescription() = default;

        // Copy assignment operator.
        DescriptorHeapDescription& operator=(const DescriptorHeapDescription& other) = default;

        // Set the type of the descriptor heap.
        void SetType(D3D12_DESCRIPTOR_HEAP_TYPE type);
        // Get the type of the descriptor heap.
        D3D12_DESCRIPTOR_HEAP_TYPE GetType() const;

        // Set the number of descriptors in the heap.
        void SetNumDescriptors(UINT num);
        // Get the number of descriptors in the heap.
        UINT GetNumDescriptors() const;

        // Set descriptor heap flags (e.g., shader visibility).
        void SetFlags(D3D12_DESCRIPTOR_HEAP_FLAGS flags);
        // Get the current descriptor heap flags.
        D3D12_DESCRIPTOR_HEAP_FLAGS GetFlags() const;

        // Set the node mask.
        void SetNodeMask(UINT mask);
        // Get the node mask.
        UINT GetNodeMask() const;

        // Get the raw D3D12 descriptor heap description.
        const D3D12_DESCRIPTOR_HEAP_DESC& GetDXDescription() const;

    private:
        // Raw D3D12 descriptor heap description.
        D3D12_DESCRIPTOR_HEAP_DESC _description;
    };
} // namespace dx12
