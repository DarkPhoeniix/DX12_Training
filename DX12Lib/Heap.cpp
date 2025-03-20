#include "DX12LibPCH.h"

#include "Heap.h"

namespace dx12
{
    Heap::Heap()
        : _heap(nullptr)
        , _description()
        , _resourceOffset(0)
    {   }

    Heap::Heap(const HeapDescription& description)
        : _heap(nullptr)
        , _description(description)
        , _resourceOffset(0)
    {   }

    Heap::~Heap()
    {
        _heap = nullptr;
    }

    void Heap::Create()
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when creating a heap");

        dx12::Device::GetDXDevice()->CreateHeap(&_description.GetDXHeapDescription(), IID_PPV_ARGS(&_heap));
        std::wstring tmp(_name.cbegin(), _name.cend());
        _heap->SetName(tmp.c_str());
    }

    void Heap::Create(const HeapDescription& description)
    {
        _description = description;
        Create();
    }

    void Heap::PlaceResource(Resource& resource, D3D12_RESOURCE_STATES state, std::uint64_t offset)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when placing resource in a heap");

        bool isDefaultHeapOffset = (offset == (std::uint64_t)-1);
        if (isDefaultHeapOffset)
        {
            offset = _resourceOffset;
        }
        ASSERT((offset + resource.GetAllocationInfo().SizeInBytes) < _description.GetSize(), "Heap is full");

        resource.CreatePlacedResource(_heap, offset, state);

        std::uint64_t size = resource.GetAllocationInfo().SizeInBytes;
        _resourceOffset += Math::AlignUp(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
    }

    void Heap::Reset()
    {
        _resourceOffset = 0;
    }

    void Heap::SetDescription(const HeapDescription& description)
    {
        _description = description;
    }

    HeapDescription Heap::GetDescription() const
    {
        return _description;
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
} // namespace dx12
