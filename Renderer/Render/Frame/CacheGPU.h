#pragma once

#include "Heap.h"

#include <mutex>

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

    DataHandle RequestPlacement(const std::string& name, uint32_t size);
    DataHandle GetOrPlaceResource(const std::string& name, uint32_t size);
    DataHandle GetResourcePlacement(const std::string& name);

    std::shared_ptr<dx12::Resource> GetCache();

private:
    std::mutex _cacheMutex;

    std::unordered_map<std::string, DataHandle> _placedResources;
    std::shared_ptr<dx12::Resource> _cache;

    uint32_t _size;
    uint32_t _currentOffset;
};
