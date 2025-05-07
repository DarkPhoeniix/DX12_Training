#pragma once

#include "HeapDescription.h"

namespace dx12
{
    class Resource;

    // Wrapper for an ID3D12Heap, representing a memory block for resource allocation.
    class Heap
    {
    public:
        // Default constructor.
        Heap();
        // Copy constructor.
        Heap(const Heap& other);
        // Move constructor.
        Heap(Heap&& other) noexcept;
        // Destructor.
        ~Heap();

        // Copy assignment operator.
        Heap& operator=(const Heap& other);
        // Move assignment operator.
        Heap& operator=(Heap&& other) noexcept;

        // Create the heap using the current description.
        void Create();
        // Create the heap with a new description.
        void Create(const HeapDescription& description);

        // Place a resource in the heap at a specified offset.
        // If offset is (UINT64)-1, the function determines the placement automatically.
        void PlaceResource(Resource& resource, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, std::uint64_t offset = (std::uint64_t)-1);

        // Reset the heap, releasing resources.
        void Reset();

        // Set a new heap description.
        void SetDescription(const HeapDescription& description);
        // Get the current heap description.
        HeapDescription GetDescription() const;

        // Set a debug name for the heap.
        void SetName(const std::string& name);
        // Get the debug name of the heap.
        const std::string& GetName() const;

        // Get the underlying DirectX 12 heap object.
        ComPtr<ID3D12Heap> GetDXHeap() const;

    private:
        // Raw DirectX 12 heap object.
        ComPtr<ID3D12Heap> _heap;
        // Description of the heap configuration.
        HeapDescription _description;

        // Offset for resource placement within the heap.
        std::uint64_t _resourceOffset;

        // Debug name of the heap.
        std::string _name;
    };
} // namespace dx12
