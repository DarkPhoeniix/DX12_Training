#include "RendererPCH.h"

#include "AllocatorPool.h"

void AllocatorPool::Init()
{
    Make(streams, 64, D3D12_COMMAND_LIST_TYPE_DIRECT);
    Make(computes, 64, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    Make(copies, 4, D3D12_COMMAND_LIST_TYPE_COPY);
}

Executor* AllocatorPool::Obtain(D3D12_COMMAND_LIST_TYPE type)
{
    std::vector<Executor>* res;
    if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        res = &streams;
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        res = &computes;
    }
    else // type == D3D12_COMMAND_LIST_TYPE_COPY
    {
        res = &copies;
    }

    for (auto& exec : *res)
    {
        if (exec.IsFree())
        {
            return &exec;
        }
    }

    LOG_CRITICAL("No free executors available for command list.");
    return nullptr;
}

void AllocatorPool::Make(std::vector<Executor>& vecExec, unsigned int size, D3D12_COMMAND_LIST_TYPE type)
{
    vecExec.resize(size);

    for (auto& exec : vecExec)
    {
        exec.Allocate(type);
        exec.SetFree(true);
    }
}
