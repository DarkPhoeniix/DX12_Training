#include "stdafx.h"

#include "CacheGPU.h"

void CacheGPU::SetHeap(std::shared_ptr<dx12::Heap> heap)
{
    Cache = heap;
    Size = heap->GetDescription().GetSize();
    CurrentOffset = 0;
}

void CacheGPU::Reset()
{
    CurrentOffset = 0;

    tempResources.clear();
}

CacheGPU::DataHandle CacheGPU::RequestPlacement(uint32_t size)
{
    DataHandle handle = {};

    uint32_t newOffset = (CurrentOffset + size);

    if (ASSERT(newOffset < Size, "GPU cache is full"))
    {
        return handle;
    }

    handle.Heap = Cache;
    handle.Offset = CurrentOffset;

    return handle;
}
