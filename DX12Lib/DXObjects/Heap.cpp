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
        ASSERT(_DXDevice, "Device is nullptr when creating a heap");

        _DXDevice->CreateHeap(&_heapDescription.GetDXHeapDescription(), IID_PPV_ARGS(&_heap));
        std::wstring tmp(_name.cbegin(), _name.cend());
        _heap->SetName(tmp.c_str());
    }

    void Heap::PlaceResource(Resource& resource, UINT64 offset)
    {
        ASSERT(_DXDevice, "Device is nullptr when placing resource in a heap");

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

    void Heap::SetName(const std::string& name)
    {
        _name = name;
        if (_heap)
        {
            std::wstring tmp(_name.cbegin(), _name.cend());
            _heap->SetName(tmp.c_str());
        }
    }

    const std::string& Heap::GetName() const
    {
        return _name;
    }

    ComPtr<ID3D12Heap> Heap::GetDXHeap() const
    {
        return _heap;
    }
} // namespace Core
