#include "RendererPCH.h"

#include "DescriptorHeapManager.h"

DescriptorHeapManager::DescriptorHeapManager(std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors)
    : _frameIndex(0)
{
    _RTVAllocator = DescriptorAllocator(0, maxRTVDescriptors);

    _DSVAllocator = DescriptorAllocator(0, maxDSVDescriptors);

    std::uint32_t heapOffset = 0;
    _staticAllocator = DescriptorAllocator(heapOffset, maxStaticDescriptors);
    heapOffset += maxStaticDescriptors;

    for (size_t i = 0; i < dx12::BACK_BUFFER_COUNT; ++i)
    {
        _dynamicAllocator.emplace_back(heapOffset, maxDynamicDescriptors);
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

DescriptorHandle DescriptorHeapManager::AllocateStatic(DescriptorHeapType type)
{
    HeapIndex index = InvalidHeapIndex;
    DescriptorHandle handle =
    {
        .Index = InvalidHeapIndex
    };

    switch (type)
    {
    case DescriptorHeapType::RTV:
        handle.Index = _RTVAllocator.Allocate();
        handle.CpuHandle = _RTVDescriptorHeap.GetCPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::DSV:
        handle.Index = _DSVAllocator.Allocate();
        handle.CpuHandle = _DSVDescriptorHeap.GetCPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::Static:
        handle.Index = _staticAllocator.Allocate();
        handle.CpuHandle = _shaderResourcesDescriptorHeap.GetCPUHandleWithOffset(handle.Index);
        handle.GpuHandle = _shaderResourcesDescriptorHeap.GetGPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::Dynamic:
        handle.Index = _dynamicAllocator[_frameIndex].Allocate();
        handle.CpuHandle = _shaderResourcesDescriptorHeap.GetCPUHandleWithOffset(handle.Index);
        handle.GpuHandle = _shaderResourcesDescriptorHeap.GetGPUHandleWithOffset(handle.Index);
        break;
    }

    return handle;
}

DescriptorHandle DescriptorHeapManager::AllocateTransient(DescriptorHeapType type)
{
    std::uint32_t index = _dynamicAllocator[_frameIndex].Allocate();
    FAIL(index != std::uint32_t(-1), "Failed to allocate transient descriptor");

    DescriptorHandle handle =
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

void DescriptorHeapManager::Reset()
{
    _RTVDescriptorHeap.Reset();
    _DSVDescriptorHeap.Reset();
    _shaderResourcesDescriptorHeap.Reset();

    _RTVAllocator.Reset();
    _DSVAllocator.Reset();
    _staticAllocator.Reset();
    for (auto& allocator : _dynamicAllocator)
    {
        allocator.Reset();
    }
}

void DescriptorHeapManager::Bind(dx12::CommandList& commandList)
{
    commandList.SetDescriptorHeaps({ _shaderResourcesDescriptorHeap.GetDXDescriptorHeap().Get() });
}

void DescriptorHeapManager::AdvanceFrameIndex()
{
    _frameIndex = (_frameIndex + 1) % dx12::BACK_BUFFER_COUNT;
}

const dx12::DescriptorHeap& DescriptorHeapManager::GetShaderResourcesDescriptorHeap() const
{
    return _shaderResourcesDescriptorHeap;
}

DescriptorHeapManager::DescriptorAllocator::DescriptorAllocator(std::uint32_t start, std::uint32_t maxDescriptors)
    : Start(start)
    , Offset(start)
    , MaxDescriptors(maxDescriptors)
    , UsedDescriptors(maxDescriptors, false)
{
}

HeapIndex DescriptorHeapManager::DescriptorAllocator::Allocate()
{
    DescriptorHandle handle;

    for (uint32_t i = 0; i < MaxDescriptors; ++i)
    {
        if (!UsedDescriptors[i])
        {
            UsedDescriptors[i] = true;

            return HeapIndex(Offset++);
        }
    }

    LOG_CRITICAL("No free descriptors available in the heap.");
    return HeapIndex(-1);
}

void DescriptorHeapManager::DescriptorAllocator::Reset()
{
    Offset = Start;
    UsedDescriptors.assign(MaxDescriptors, false);
}
