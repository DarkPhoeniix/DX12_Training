#include "RendererPCH.h"

#include "FencePool.h"

FencePool::FencePool(rhi::Device* device)
    : _device(device)
{
}

void FencePool::Init()
{
    // TODO: that sucks
    _fences.resize(128 + 128 + 4); // Direct + Compute + Copy

    for (auto& fence : _fences)
    {
        fence = _device->CreateFence(0);
    }
}

rhi::Fence* FencePool::Obtain()
{
    for (auto& fence : _fences)
    {
        if (fence->IsFree())
        {
            return fence.get();
        }
    }

    return nullptr;
}
