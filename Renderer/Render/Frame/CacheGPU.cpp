#include "stdafx.h"

#include "CacheGPU.h"

void CacheGPU::SetHeap(std::shared_ptr<dx12::Heap> heap)
{
    Cache = heap;
    Size = heap->GetDescription().GetSize();
    CurrentOffset = 0;
}

void CacheGPU::Clear()
{
    CurrentOffset = 0;

    CachedResources.clear();
}

CacheGPU::DataHandle CacheGPU::PlaceResource(dx12::Resource&& resource)
{
    uint32_t resourceSize = resource.GetResourceDescription().GetSize().x * resource.GetResourceDescription().GetSize().y;
    DataHandle heapHandle = RequestPlacement(resourceSize);

    resource.CreatePlacedResource(heapHandle.Heap->GetDXHeap(), heapHandle.Offset);

    CachedResources.push_back(std::move(resource));

    return heapHandle;
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
