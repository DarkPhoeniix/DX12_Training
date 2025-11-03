#include "RHI_PCH.h"

#include "Heap.h"

namespace dx12
{
    Heap::Heap()
        : _heap(nullptr)
        , _description()
        , _resourceOffset(0)
    {
    }

    Heap::Heap(const Heap& other)
        : _heap(other._heap)
        , _description(other._description)
        , _resourceOffset(other._resourceOffset)
    {
    }

    Heap::Heap(Heap&& other) noexcept
        : _heap(std::move(other._heap))
        , _description(std::move(other._description))
        , _resourceOffset(other._resourceOffset)
    {
    }

    Heap::~Heap()
    {
        _heap = nullptr;
    }

    Heap& Heap::operator=(const Heap& other)
    {
        if (this != &other)
        {
            _heap = other._heap;
            _description = other._description;
            _resourceOffset = other._resourceOffset;
        }

        return *this;
    }

    Heap& Heap::operator=(Heap&& other) noexcept
    {
        if (this != &other)
        {
            _heap = std::move(other._heap);
            _description = std::move(other._description);
            _resourceOffset = other._resourceOffset;
        }

        return *this;
    }

    void Heap::Create()
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when creating a heap.");

        HRESULT result = dx12::Device::GetDXDevice()->CreateHeap(&_description.GetDXHeapDescription(), IID_PPV_ARGS(&_heap));
        CHECK(result, "Failed to create heap.");

        std::wstring tmp(_name.cbegin(), _name.cend());
        _heap->SetName(tmp.c_str());
    }

    void Heap::Create(const HeapDescription& description)
    {
        _description = description;
        Create();
    }

    void Heap::PlaceResource(Resource& resource, ResourceState state, std::uint64_t offset)
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when placing resource in a heap.");

        bool isDefaultHeapOffset = (offset == (std::uint64_t)-1);
        if (isDefaultHeapOffset)
        {
            offset = _resourceOffset;
        }
        std::uint64_t size = resource.GetAllocationInfo().SizeInBytes;
        ASSERT((offset + size) <= _description.GetSize(), "Heap is full.");

        resource.CreatePlacedResource(_heap, offset, state);

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
