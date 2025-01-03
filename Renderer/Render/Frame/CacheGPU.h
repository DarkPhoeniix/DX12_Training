#pragma once

#include "Heap.h"

class CacheGPU
{
public:
    struct DataHandle
    {
        void* DataCPU = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS DataGPU = 0;
        uint32_t Offset = -1;
    };

    void SetResource(std::shared_ptr<dx12::Resource> memoryBlock);
    void Clear();

    DataHandle RequestPlacement(uint32_t size);

    std::shared_ptr<dx12::Resource> Cache;

    uint32_t Size;
    uint32_t CurrentOffset;
};
