#include "RendererPCH.h"

#include "CacheGPU.h"

void CacheGPU::SetResource(std::shared_ptr<dx12::Resource> memoryBlock)
{
    Cache = memoryBlock;
    Size = memoryBlock->GetResourceDescription().GetSize().x * memoryBlock->GetResourceDescription().GetSize().y;
    CurrentOffset = 0;
}

void CacheGPU::Clear()
{
    CurrentOffset = 0;
}

CacheGPU::DataHandle CacheGPU::RequestPlacement(uint32_t size)
{
    DataHandle handle = {};

    uint32_t newOffset = (CurrentOffset + size);

    if (ASSERT(newOffset < Size, "GPU cache is full"))  
    {
        return handle;
    }

    handle.DataCPU = Cache->Map(CurrentOffset, 0); // TODO: not sure if 0 will work good
    handle.DataGPU = Cache->OffsetGPU(CurrentOffset);
    handle.Offset = CurrentOffset;

    return handle;
}
