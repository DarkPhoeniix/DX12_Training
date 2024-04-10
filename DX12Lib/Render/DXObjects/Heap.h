#pragma once

#include "HeapDescription.h"

class Heap
{
public:
    Heap();
    Heap(const HeapDescription& heapDescription);
    ~Heap();

    void Create();
    void PlaceResource(Resource& resource, UINT64 offset = (UINT64)-1);

    void SetDevice(ComPtr<ID3D12Device> device);

private:
    ComPtr<ID3D12Heap> _heap;
    HeapDescription _heapDescription;

    UINT64 _resourceOffset;

    ComPtr<ID3D12Device> _device;
};
