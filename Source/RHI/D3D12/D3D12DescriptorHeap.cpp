
#include "RHI_PCH.h"

#include "D3D12DescriptorHeap.h"

#include "D3D12Device.h"
#include "D3D12Descriptor.h"
#include "D3D12Helpers.h"

namespace rhi::d3d12
{
    namespace
    {
        constexpr D3D12_DESCRIPTOR_HEAP_TYPE ToD3D12DescriptorHeapType(rhi::DescriptorHeapType type)
        {
            switch (type)
            {
            case rhi::DescriptorHeapType::RTV:
                return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case rhi::DescriptorHeapType::DSV:
                return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            case rhi::DescriptorHeapType::CBV_SRV_UAV:
                return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            default:
                UNREACHABLE("Invalid descriptor heap type!");
                return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            }
        }
    } // namespace unnamed

    D3D12DescriptorHeap::D3D12DescriptorHeap(rhi::Device* device, const rhi::DescriptorHeapDescription& description, const std::string& name)
        : _description(description)
        , _device(device)
        , _currentOffset(0)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = 
        {
            .Type = ToD3D12DescriptorHeapType(description.Type),
            .NumDescriptors = description.NumDescriptors,
            .Flags = description.ShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };

        NativeDevice* d3d12DeviceNative = D3D12Cast<NativeDevice>(device->GetNative());

        HRESULT result = d3d12DeviceNative->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descriptorHeap));
        CHECK(result, "Failed to create D3D12DescriptorHeap.");

        _heapIncrementSize = d3d12DeviceNative->GetDescriptorHandleIncrementSize(ToD3D12DescriptorHeapType(description.Type));

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_descriptorHeap.Get(), name);
#endif // ENABLE_DEBUG_NAMES
    }

    D3D12DescriptorHeap::D3D12DescriptorHeap(D3D12DescriptorHeap&& other) noexcept
        : _descriptorHeap(std::move(other._descriptorHeap))
        , _description(std::move(other._description))
        , _heapIncrementSize(other._heapIncrementSize)
        , _currentOffset(other._currentOffset)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(other._name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12DescriptorHeap::~D3D12DescriptorHeap()
    {
    }

    D3D12DescriptorHeap& D3D12DescriptorHeap::operator=(D3D12DescriptorHeap&& other) noexcept
    {
        if (this != &other)
        {
            _descriptorHeap = std::move(other._descriptorHeap);
            _description = std::move(other._description);
            _heapIncrementSize = other._heapIncrementSize;
            _currentOffset = other._currentOffset;
            _device = std::move(other._device);
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    void D3D12DescriptorHeap::Reset()
    {
        _currentOffset = 0;
    }

    std::uint32_t D3D12DescriptorHeap::CopyResourceDescriptor(rhi::CPUDescriptor descriptor)
    {
        ASSERT((_currentOffset + 1) < _description.NumDescriptors, "Descriptor heap is full, cannot copy descriptor");

        D3D12_CPU_DESCRIPTOR_HANDLE handle = _descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _heapIncrementSize * _currentOffset;

        NativeDevice* d3d12DeviceNative = D3D12Cast<NativeDevice>(_device->GetNative());
        d3d12DeviceNative->CopyDescriptorsSimple(1, handle, 
            ToD3D12Handle(descriptor),
            ToD3D12DescriptorHeapType(_description.Type));

        return _currentOffset++;
    }

    rhi::CPUDescriptor D3D12DescriptorHeap::GetHeapStartCPUHandle()
    {
        return ToRHIHandle(_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }

    rhi::GPUDescriptor D3D12DescriptorHeap::GetHeapStartGPUHandle()
    {
        return ToRHIHandle(_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
    }

    rhi::CPUDescriptor D3D12DescriptorHeap::GetCPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.NumDescriptors, "Offset is out of bounds for descriptor heap");

        rhi::CPUDescriptor descriptor = ToRHIHandle(_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
        descriptor.Offset(offset * _heapIncrementSize);

        return descriptor;
    }

    rhi::GPUDescriptor D3D12DescriptorHeap::GetGPUHandleWithOffset(std::uint32_t offset)
    {
        ASSERT(offset < _description.NumDescriptors, "Offset is out of bounds for descriptor heap");

        rhi::GPUDescriptor descriptor = ToRHIHandle(_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
        descriptor.Offset(offset * _heapIncrementSize);

        return descriptor;
    }

    std::uint32_t D3D12DescriptorHeap::Offset()
    {
        ASSERT((_currentOffset + 1) < _description.NumDescriptors, "Descriptor heap is full, cannot get offset");

        std::uint32_t offset = _currentOffset;
        ++_currentOffset;

        return offset;
    }

    std::uint32_t D3D12DescriptorHeap::GetCurrentOffset() const
    {
        return _currentOffset;
    }

    void* D3D12DescriptorHeap::GetNative() const
    {
        return static_cast<void*>(_descriptorHeap.Get());
    }
} // namespace rhi::d3d12
