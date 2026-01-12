#include "RendererPCH.h"

#include "FencePool.h"

void FencePool::Init()
{
    // TODO: that sucks
    fences.resize(128 + 128 + 4); // Direct + Compute + Copy

    for (dx12::Fence& fence : fences)
    {
        fence.Init();
        fence.SetFree(true);
    }
}

dx12::Fence* FencePool::Obtain()
{
    for (dx12::Fence& fence : fences)
    {
        if (fence.IsFree())
        {
            return &fence;
        }
    }

    return nullptr;
}
