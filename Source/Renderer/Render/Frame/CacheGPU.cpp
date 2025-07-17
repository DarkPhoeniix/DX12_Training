#include "RendererPCH.h"

#include "CacheGPU.h"

void CacheGPU::SetResource(std::shared_ptr<dx12::Resource> memoryBlock)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);

    ASSERT(memoryBlock, "Trying to set a null memory block in GPU cache");

    _cache = memoryBlock;
    _size = memoryBlock->GetResourceDescription().GetSize().x * memoryBlock->GetResourceDescription().GetSize().y;
    _currentOffset = 0;
}

void CacheGPU::Clear()
{
    std::unique_lock<std::shared_mutex> lock(_mutex);

    ASSERT(_cache, "Trying to clear GPU cache without setting a memory block");

    _currentOffset = 0;
    _placedResources.clear();
}

CacheGPU::DataHandle CacheGPU::RequestPlacement(const std::string& name, std::uint32_t size)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);

    std::uint32_t newOffset = Math::AlignUp((_currentOffset + size), 256);
    ASSERT(newOffset <= _size, "Requested size exceeds GPU cache size");

    DataHandle handle;
    handle.DataCPU = _cache->Map<char>() + _currentOffset;
    handle.DataGPU = _cache->OffsetGPU(_currentOffset);
    handle.Offset = _currentOffset;

    _placedResources.emplace(name, handle);
    _currentOffset = newOffset;

    return handle;
}

CacheGPU::DataHandle CacheGPU::GetOrPlaceResource(const std::string& name, std::uint32_t size)
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
    std::shared_lock<std::shared_mutex> lock(_mutex);

    auto it = _placedResources.find(name);
    if (it == _placedResources.end())
    {
        FAIL("Failed to retrieve cached data for \"{}\", resource not found", name);
        return {};
    }

    return it->second;
}

std::shared_ptr<dx12::Resource> CacheGPU::GetCache()
{
    return _cache;
}
