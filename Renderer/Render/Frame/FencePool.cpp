#include "stdafx.h"

#include "FencePool.h"

void FencePool::Init()
{
    fences.resize(32 + 32 + 4); // Direct + Compute + Copy

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
