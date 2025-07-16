#include "DX12LibPCH.h"

#include "DescriptorHeap.h"

namespace dx12
{
    DescriptorHeap::DescriptorHeap()
        : _descriptorHeap(nullptr)
        , _description{}
        , _heapIncrementSize(0)
        , _currentOffset(0)
    {   }

    DescriptorHeap::DescriptorHeap(const DescriptorHeapDescription& description)
        : _descriptorHeap(nullptr)
        , _description(description)
        , _heapIncrementSize(0)
        , _currentOffset(0)
    {   }

    DescriptorHeap::DescriptorHeap(const DescriptorHeap& other)
        : _descriptorHeap(other._descriptorHeap)
        , _description(other._description)
        , _heapIncrementSize(other._heapIncrementSize)
        , _currentOffset(other._currentOffset)
    {
    }

    DescriptorHeap::DescriptorHeap(DescriptorHeap&& other) noexcept
        : _descriptorHeap(std::move(other._descriptorHeap))
        , _description(std::move(other._description))
        , _heapIncrementSize(other._heapIncrementSize)
        , _currentOffset(other._currentOffset)
    {
    }

    DescriptorHeap::~DescriptorHeap()
    {
        _descriptorHeap = nullptr;
    }

    DescriptorHeap& DescriptorHeap::operator=(const DescriptorHeap& other)
    {
        if (this != &other)
        {
            _descriptorHeap = other._descriptorHeap;
            _description = other._description;
            _heapIncrementSize = other._heapIncrementSize;
            _currentOffset = other._currentOffset;
        }

        return *this;
    }

    DescriptorHeap& DescriptorHeap::operator=(DescriptorHeap&& other) noexcept
    {
        if (this != &other)
        {
            _descriptorHeap = std::move(other._descriptorHeap);
            _description = std::move(other._description);
            _heapIncrementSize = other._heapIncrementSize;
            _currentOffset = other._currentOffset;
        }

        return *this;
    }

    void DescriptorHeap::Create()
    {
        ASSERT(dx12::Device::GetDXDevice(), "Device is nullptr when trying to create descriptor heap");

        HRESULT result = dx12::Device::GetDXDevice()->CreateDescriptorHeap(&_description.GetDXDescription(), IID_PPV_ARGS(&_descriptorHeap));
        CHECK(result, "Failed to create descriptor heap.");

        std::wstring tmp(_name.begin(), _name.end());
        _descriptorHeap->SetName(tmp.c_str());

        _heapIncrementSize = dx12::Device::GetDXDevice()->GetDescriptorHandleIncrementSize(_description.GetType());
    }

    void DescriptorHeap::Create(const DescriptorHeapDescription& description)
    {
        _description = description;

        Create();
    }

    void DescriptorHeap::Reset()
    {
        _currentOffset = 0;
    }

    std::uint32_t DescriptorHeap::CopyResourceDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        ASSERT((_currentOffset + 1) < _description.GetNumDescriptors(), "Descriptor heap is full, cannot copy descriptor");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * _currentOffset;

        dx12::Device::GetDXDevice()->CopyDescriptorsSimple(1, handle, descriptor, _description.GetType());

        return _currentOffset++;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartCPUHandle()
    {
        return _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetHeapStartGPUHandle()
    {
        return _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.GetNumDescriptors(), "Offset is out of bounds for descriptor heap");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += (offset * _heapIncrementSize);
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.GetNumDescriptors(), "Offset is out of bounds for descriptor heap");

        D3D12_GPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += (offset * _heapIncrementSize);
        return handle;
    }

    std::uint32_t DescriptorHeap::Offset()
    {
        ASSERT((_currentOffset + 1) < _description.GetNumDescriptors(), "Descriptor heap is full, cannot get offset");

        std::uint32_t offset = _currentOffset;
        ++_currentOffset;
        return offset;
    }

    std::uint32_t DescriptorHeap::GetCurrentOffset() const
    {
        return _currentOffset;
    }

    void DescriptorHeap::SetDescription(const DescriptorHeapDescription& description)
    {
        _description = description;
    }

    const DescriptorHeapDescription& DescriptorHeap::GetDescription() const
    {
        return _description;
    }

    void DescriptorHeap::SetName(const std::string& name)
    {
        _name = name;
        if (_descriptorHeap)
        {
            std::wstring tmp(_name.cbegin(), _name.cend());
            _descriptorHeap->SetName(tmp.c_str());
        }
    }

    const std::string& DescriptorHeap::GetName() const
    {
        return _name;
    }

    ComPtr<ID3D12DescriptorHeap> DescriptorHeap::GetDXDescriptorHeap() const
    {
        return _descriptorHeap;
    }
} // namespace dx12
