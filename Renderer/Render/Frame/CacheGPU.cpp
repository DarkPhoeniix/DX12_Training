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

    uint32_t newOffset = Math::AlignUp((CurrentOffset + size), 256);

    if (ASSERT(newOffset < Size, "GPU cache is full"))  
    {
        return handle;
    }

    handle.DataCPU = (char*)Cache->Map() + CurrentOffset;
    handle.DataGPU = Cache->OffsetGPU(CurrentOffset);
    handle.Offset = CurrentOffset;

    CurrentOffset = newOffset;

    return handle;
}
