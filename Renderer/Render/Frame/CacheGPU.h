#pragma once

#include "Heap.h"

class CacheGPU
{
public:
    struct DataHandle
    {
        std::shared_ptr<dx12::Heap> Heap = nullptr;
        uint32_t Offset = -1;
    };

    void SetHeap(std::shared_ptr<dx12::Heap> heap);
    void Clear();

    DataHandle RequestPlacement(uint32_t size);
    DataHandle PlaceResource(dx12::Resource&& resource);

    std::shared_ptr<dx12::Heap> Cache;

    uint32_t Size;
    uint32_t CurrentOffset;

    std::vector<dx12::Resource> CachedResources;
};
