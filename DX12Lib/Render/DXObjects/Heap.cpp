#include "stdafx.h"

#include "Heap.h"

Heap::Heap()
    : _heap(nullptr)
    , _heapDescription()
    , _resourceOffset(0)
    , _device(nullptr)
{
}

Heap::Heap(const HeapDescription& heapDescription)
    : _heap(nullptr)
    , _heapDescription(heapDescription)
    , _resourceOffset(0)
    , _device(nullptr)
{
}

Heap::~Heap()
{
    if (_heap)
        _heap = nullptr;

    _device = nullptr;
}

void Heap::Create()
{
    if (!_device)
        Logger::Log(LogType::Error, "Device is nullptr when creating a heap");

    _device->CreateHeap(&_heapDescription.GetDXHeapDescription(), IID_PPV_ARGS(&_heap));
}

void Heap::PlaceResource(Resource& resource, UINT64 offset)
{
    if (!_device)
        Logger::Log(LogType::Error, "Device is nullptr when placing resource in a heap");

    bool isDefaultHeapOffset = (offset == (UINT64)-1);
    if (isDefaultHeapOffset)
        offset = _resourceOffset;

    resource.CreatePlacedResource(_heap, offset);

    unsigned int size = resource.GetResourceDescription().GetSize().x * resource.GetResourceDescription().GetSize().y;
    _resourceOffset += Math::AlignUp(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
}

void Heap::SetDevice(ComPtr<ID3D12Device> device)
{
    _device = device;
}
