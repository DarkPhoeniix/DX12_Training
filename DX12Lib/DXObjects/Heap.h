#pragma once

#include "HeapDescription.h"

class Resource;

class Heap
{
public:
    Heap();
    Heap(const HeapDescription& heapDescription);
    ~Heap();

    void SetDevice(ComPtr<ID3D12Device2> device);
    ComPtr<ID3D12Device2> GetDevice() const;

    void Create();
    void PlaceResource(Resource& resource, UINT64 offset = (UINT64)-1);

private:
    ComPtr<ID3D12Heap> _heap;
    HeapDescription _heapDescription;

    UINT64 _resourceOffset;

    ComPtr<ID3D12Device2> _DXDevice;
};
