#pragma once

#include <shared_mutex>

class CacheGPU
{
public:
    struct DataHandle
    {
        void* DataCPU = nullptr;
        std::uint64_t DataGPU = 0;
        std::uint32_t Offset = (std::uint32_t)-1;
    };

    void SetResource(std::shared_ptr<rhi::Buffer> memoryBlock);
    void Clear();

    DataHandle RequestPlacement(const std::string& name, std::uint32_t size);
    DataHandle GetOrPlaceResource(const std::string& name, std::uint32_t size);
    DataHandle GetResourcePlacement(const std::string& name);

    std::shared_ptr<rhi::Buffer> GetCache();

private:
    std::unordered_map<std::string, DataHandle> _placedResources;
    std::shared_ptr<rhi::Buffer> _cache;

    std::uint32_t _size;
    std::uint32_t _currentOffset;

    std::shared_mutex _mutex;
};
