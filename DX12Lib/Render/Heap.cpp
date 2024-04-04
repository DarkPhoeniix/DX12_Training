#include "stdafx.h"
#include "Heap.h"

#include "Application.h"

Heap::Heap()
    : _heap(nullptr)
    , _heapDescription()
{
}

Heap::Heap(const HeapDescription& heapDescription)
    : _heapDescription(heapDescription)
{
}

Heap::~Heap()
{
    if (_heap)
        _heap->Release();
}

void Heap::Create()
{
    auto device = Application::get().getDevice();
    D3D12_HEAP_DESC desc = _heapDescription.GetDXHeapDescription();
    device->CreateHeap(&desc, IID_PPV_ARGS(&_heap));
}

void Heap::PlaceResource(Resource& resource, UINT64 offset)
{
    auto device = Application::get().getDevice();

    bool isDefaultHeapOffset = (offset == (UINT64)-1);
    if (isDefaultHeapOffset)
        offset = _resourceOffset;

    resource.CreatePlacedResource(_heap, offset);
}
