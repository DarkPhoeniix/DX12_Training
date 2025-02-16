#include "RendererPCH.h"

#include "CacheGPU.h"

void CacheGPU::SetResource(std::shared_ptr<dx12::Resource> memoryBlock)
{
    _cache = memoryBlock;
    _size = memoryBlock->GetResourceDescription().GetSize().x * memoryBlock->GetResourceDescription().GetSize().y;
    _currentOffset = 0;
}

void CacheGPU::Clear()
{
    _currentOffset = 0;
    _placedResources.clear();
}

CacheGPU::DataHandle CacheGPU::RequestPlacement(const std::string& name, uint32_t size)
{
    DataHandle handle = {};

    uint32_t newOffset = Math::AlignUp((_currentOffset + size), 256);

    if (ASSERT(newOffset < _size, "GPU cache is full"))  
    {
        return handle;
    }

    handle.DataCPU = (char*)_cache->Map() + _currentOffset;
    handle.DataGPU = _cache->OffsetGPU(_currentOffset);
    handle.Offset = _currentOffset;

    _placedResources.emplace(name, handle);
    _currentOffset = newOffset;

    return handle;
}

CacheGPU::DataHandle CacheGPU::GetOrPlaceResource(const std::string& name, uint32_t size)
{
    DataHandle handle = GetResourcePlacement(name);

    if (!handle.DataCPU)
    {
        handle = RequestPlacement(name, size);
    }

    return handle;
}

CacheGPU::DataHandle CacheGPU::GetResourcePlacement(const std::string& name)
{
    DataHandle handle = {};

    auto it = _placedResources.find(name);
    if (it != _placedResources.end())
    {
        handle = it->second;
    }

    return handle;
}

std::shared_ptr<dx12::Resource> CacheGPU::GetCache()
{
    return _cache;
}
