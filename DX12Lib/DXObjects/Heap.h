#pragma once

#include "HeapDescription.h"

namespace Core
{
    class Resource;

    class Heap
    {
    public:
        Heap();
        Heap(const HeapDescription& heapDescription);
        ~Heap();

        void Create();
        void PlaceResource(Resource& resource, UINT64 offset = (UINT64)-1);

        void SetDescription(const HeapDescription& description);
        HeapDescription GetDescription() const;

        void SetName(const std::string& name);
        const std::string& GetName() const;

        ComPtr<ID3D12Heap> GetDXHeap() const;

    private:
        ComPtr<ID3D12Device2> _DXDevice;

        ComPtr<ID3D12Heap> _heap;
        HeapDescription _heapDescription;

        UINT64 _resourceOffset;

        std::string _name;
    };
} // namespace Core
