#include "stdafx.h"

#include "FencePool.h"

void FencePool::Init()
{
    fences.resize(32 + 32 + 4); // Direct + Compute + Copy

    for (Core::Fence& fence : fences)
    {
        fence.Init();
        fence.SetFree(true);
    }
}

Core::Fence* FencePool::Obtain()
{
    for (Core::Fence& fence : fences)
    {
        if (fence.IsFree())
        {
            return &fence;
        }
    }

    return nullptr;
}
