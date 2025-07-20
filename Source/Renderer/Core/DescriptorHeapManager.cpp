#include "RendererPCH.h"

#include "DescriptorHeapManager.h"

DescriptorHeapManager::DescriptorHeapManager(std::uint64_t maxRTVDescriptors, std::uint64_t maxDSVDescriptors, std::uint64_t maxStaticDescriptors, std::uint64_t maxDynamicDescriptors)
    : _frameIndex(0)
{
    _RTVAllocator = DescriptorAllocator(0, maxRTVDescriptors);

    _DSVAllocator = DescriptorAllocator(0, maxDSVDescriptors);

    std::uint32_t heapOffset = 0;
    _staticAllocator = DescriptorAllocator(heapOffset, maxStaticDescriptors);
    heapOffset += maxStaticDescriptors;

    for (size_t i = 0; i < dx12::BACK_BUFFER_COUNT; ++i)
    {
        _dynamicAllocator[i] = DescriptorAllocator(heapOffset, maxDynamicDescriptors);
        heapOffset += maxDynamicDescriptors;
    }

    dx12::DescriptorHeapDescription rtvHeapDesc;
    rtvHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    rtvHeapDesc.SetNumDescriptors(maxRTVDescriptors);
    _RTVDescriptorHeap.Create(rtvHeapDesc);
    _RTVDescriptorHeap.SetName("RTV Descriptor Heap");

    dx12::DescriptorHeapDescription dsvHeapDesc;
    dsvHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    dsvHeapDesc.SetNumDescriptors(maxDSVDescriptors);
    _DSVDescriptorHeap.Create(dsvHeapDesc);
    _DSVDescriptorHeap.SetName("DSV Descriptor Heap");

    dx12::DescriptorHeapDescription buffersHeapDesc;
    buffersHeapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    buffersHeapDesc.SetNumDescriptors(maxStaticDescriptors + maxDynamicDescriptors * dx12::BACK_BUFFER_COUNT);
    buffersHeapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    _shaderResourcesDescriptorHeap.Create(buffersHeapDesc);
    _shaderResourcesDescriptorHeap.SetName("Shader Resources Descriptor Heap");
}

dx12::DescriptorHandle DescriptorHeapManager::AllocateStatic()
{
    std::uint64_t index = _staticAllocator.Allocate();
    FAIL(index != std::uint64_t(-1), "Failed to allocate static descriptor");

    dx12::DescriptorHandle handle =
    {
        .Index = index,
        .CpuHandle = _shaderResourcesDescriptorHeap.GetCPUHandleWithOffset(index),
        .GpuHandle = _shaderResourcesDescriptorHeap.GetGPUHandleWithOffset(index)
    };

    return handle;
}

dx12::DescriptorHandle DescriptorHeapManager::AllocateTransient()
{
    std::uint64_t index = _dynamicAllocator[_frameIndex].Allocate();
    FAIL(index != std::uint64_t(-1), "Failed to allocate transient descriptor");

    dx12::DescriptorHandle handle =
    {
        .Index = index,
        .CpuHandle = _shaderResourcesDescriptorHeap.GetCPUHandleWithOffset(index),
        .GpuHandle = _shaderResourcesDescriptorHeap.GetGPUHandleWithOffset(index)
    };

    return handle;
}

void DescriptorHeapManager::ResetTransient()
{
    _dynamicAllocator[_frameIndex].Reset();
}

void DescriptorHeapManager::Bind(dx12::CommandList& commandList)
{
    commandList.SetDescriptorHeaps({ _shaderResourcesDescriptorHeap.GetDXDescriptorHeap().Get() });
}

void DescriptorHeapManager::AdvanceFrameIndex()
{
    _frameIndex = (_frameIndex + 1) % dx12::BACK_BUFFER_COUNT;
}

DescriptorHeapManager::DescriptorAllocator::DescriptorAllocator(std::uint64_t offset, std::uint64_t maxDescriptors)
    : Offset(offset)
    , MaxDescriptors(maxDescriptors)
    , UsedDescriptors(maxDescriptors, false)
{
}

std::uint64_t DescriptorHeapManager::DescriptorAllocator::Allocate()
{
    dx12::DescriptorHandle handle;

    for (uint32_t i = 0; i < MaxDescriptors; ++i)
    {
        if (!UsedDescriptors[i])
        {
            UsedDescriptors[i] = true;

            return (Offset + i);
        }
    }

    LOG_CRITICAL("No free descriptors available in the heap.");
    return std::uint64_t(-1);
}

void DescriptorHeapManager::DescriptorAllocator::Reset()
{
    UsedDescriptors.assign(MaxDescriptors, false);
}
