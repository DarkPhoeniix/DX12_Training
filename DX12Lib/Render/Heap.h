#pragma once

#include "HeapDescription.h"

class Heap
{
public:
    Heap();
    Heap(const HeapDescription& heapDescription);
    ~Heap();

    void Create();
    void PlaceResource(Resource& resource, UINT64 offset);

private:
    ComPtr<ID3D12Heap> _heap;
    HeapDescription _heapDescription;

    UINT64 _resourceOffset;
};
