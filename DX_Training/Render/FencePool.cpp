#include "stdafx.h"

#include "FencePool.h"

void FencePool::Init(ComPtr<ID3D12Device> device)
{
    fences.resize(32 + 32 + 4); // Direct + Compute + Copy

    for (Fence& fence : fences)
    {
        fence.Init(device);
        fence.SetFree(true);
    }
}

Fence* FencePool::Obtain()
{
    for (Fence& fence : fences)
    {
        if (fence.IsFree())
        {
            return &fence;
        }
    }

    return nullptr;
}
