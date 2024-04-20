#include "stdafx.h"

#include "Heap.h"

namespace Core
{
    Heap::Heap()
        : _DXDevice(Core::Device::GetDXDevice())
        , _heap(nullptr)
        , _heapDescription()
        , _resourceOffset(0)
    {   }

    Heap::Heap(const HeapDescription& heapDescription)
        : _DXDevice(Core::Device::GetDXDevice())
        , _heap(nullptr)
        , _heapDescription(heapDescription)
        , _resourceOffset(0)
    {   }

    Heap::~Heap()
    {
        _DXDevice = nullptr;
        _heap = nullptr;
    }

    void Heap::Create()
    {
        if (!_DXDevice)
        {
            Logger::Log(LogType::Error, "Device is nullptr when creating a heap");
        }

        _DXDevice->CreateHeap(&_heapDescription.GetDXHeapDescription(), IID_PPV_ARGS(&_heap));
    }

    void Heap::PlaceResource(Resource& resource, UINT64 offset)
    {
        if (!_DXDevice)
        {
            Logger::Log(LogType::Error, "Device is nullptr when placing resource in a heap");
        }

        bool isDefaultHeapOffset = (offset == (UINT64)-1);
        if (isDefaultHeapOffset)
        {
            offset = _resourceOffset;
        }

        resource.CreatePlacedResource(_heap, offset);

        unsigned int size = resource.GetResourceDescription().GetSize().x * resource.GetResourceDescription().GetSize().y;
        _resourceOffset += Math::AlignUp(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
    }

    void Heap::SetDescription(const HeapDescription& description)
    {
        _heapDescription = description;
    }

    HeapDescription Heap::GetDescription() const
    {
        return _heapDescription;
    }
} // namespace Core
