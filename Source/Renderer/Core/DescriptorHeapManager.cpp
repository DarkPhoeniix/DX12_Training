#include "RendererPCH.h"

#include "DescriptorHeapManager.h"

#include "RHI/SwapChain.h"

std::unique_ptr<DescriptorHeapManager> DescriptorHeapManager::_instance = nullptr;

DescriptorHeapManager::DescriptorHeapManager(rhi::Device* device, std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors)
    : _frameIndex(0)
    , _device(device)
{
    _RTVAllocator = DescriptorAllocator(0, maxRTVDescriptors);

    _DSVAllocator = DescriptorAllocator(0, maxDSVDescriptors);

    std::uint32_t heapOffset = 0;
    _staticAllocator = DescriptorAllocator(heapOffset, maxStaticDescriptors);
    heapOffset += maxStaticDescriptors;

    for (size_t i = 0; i < rhi::BACK_BUFFER_COUNT; ++i)
    {
        _dynamicAllocator.emplace_back(heapOffset, maxDynamicDescriptors);
        heapOffset += maxDynamicDescriptors;
    }

    rhi::DescriptorHeapDescription rtvHeapDesc =
    {
        .Type = rhi::DescriptorHeapType::RTV,
        .NumDescriptors = maxRTVDescriptors,
        .ShaderVisible = false
    };
    _RTVDescriptorHeap = _device->CreateDescriptorHeap(rtvHeapDesc, "RTV Descriptor Heap");

    rhi::DescriptorHeapDescription dsvHeapDesc =
    {
        .Type = rhi::DescriptorHeapType::DSV,
        .NumDescriptors = maxDSVDescriptors,
        .ShaderVisible = false
    };
    _DSVDescriptorHeap = _device->CreateDescriptorHeap(dsvHeapDesc, "DSV Descriptor Heap");

    rhi::DescriptorHeapDescription buffersHeapDesc =
    {
        .Type = rhi::DescriptorHeapType::CBV_SRV_UAV,
        .NumDescriptors = maxStaticDescriptors + maxDynamicDescriptors * rhi::BACK_BUFFER_COUNT,
        .ShaderVisible = true
    };
    _shaderResourcesDescriptorHeap = _device->CreateDescriptorHeap(buffersHeapDesc, "Shader Resources Descriptor Heap");
}

void DescriptorHeapManager::Create(rhi::Device* device, std::uint32_t maxRTVDescriptors, std::uint32_t maxDSVDescriptors, std::uint32_t maxStaticDescriptors, std::uint32_t maxDynamicDescriptors)
{
    if (_instance)
    {
        LOG_WARNING("DescriptorHeapManager instance already exists. Creation skipped.");
        return;
    }
    _instance = std::unique_ptr<DescriptorHeapManager>(new DescriptorHeapManager(device, maxRTVDescriptors, maxDSVDescriptors, maxStaticDescriptors, maxDynamicDescriptors));
}

void DescriptorHeapManager::Destroy()
{
    if (_instance)
    {
        _instance.reset();
    }
    else
    {
        LOG_WARNING("DescriptorHeapManager instance does not exist. Destruction skipped.");
    }
}

DescriptorHeapManager& DescriptorHeapManager::Get()
{
    ASSERT(_instance, "DescriptorHeapManager instance is not created. Call Create() first.");
    return *_instance;
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
        handle.CpuHandle = _RTVDescriptorHeap->GetCPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::DSV:
        handle.Index = _DSVAllocator.Allocate();
        handle.CpuHandle = _DSVDescriptorHeap->GetCPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::Static:
        handle.Index = _staticAllocator.Allocate();
        handle.CpuHandle = _shaderResourcesDescriptorHeap->GetCPUHandleWithOffset(handle.Index);
        handle.GpuHandle = _shaderResourcesDescriptorHeap->GetGPUHandleWithOffset(handle.Index);
        break;
    case DescriptorHeapType::Dynamic:
        handle.Index = _dynamicAllocator[_frameIndex].Allocate();
        handle.CpuHandle = _shaderResourcesDescriptorHeap->GetCPUHandleWithOffset(handle.Index);
        handle.GpuHandle = _shaderResourcesDescriptorHeap->GetGPUHandleWithOffset(handle.Index);
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
        .CpuHandle = _shaderResourcesDescriptorHeap->GetCPUHandleWithOffset(index),
        .GpuHandle = _shaderResourcesDescriptorHeap->GetGPUHandleWithOffset(index)
    };

    return handle;
}

void DescriptorHeapManager::ResetTransient()
{
    _dynamicAllocator[_frameIndex].Reset();
}

void DescriptorHeapManager::Reset()
{
    _RTVDescriptorHeap->Reset();
    _DSVDescriptorHeap->Reset();
    _shaderResourcesDescriptorHeap->Reset();

    _RTVAllocator.Reset();
    _DSVAllocator.Reset();
    _staticAllocator.Reset();
    for (auto& allocator : _dynamicAllocator)
    {
        allocator.Reset();
    }
}

void DescriptorHeapManager::AdvanceFrameIndex()
{
    _frameIndex = (_frameIndex + 1) % rhi::BACK_BUFFER_COUNT;
}

rhi::DescriptorHeap* DescriptorHeapManager::GetShaderResourcesDescriptorHeap() const
{
    return _shaderResourcesDescriptorHeap.get();
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
