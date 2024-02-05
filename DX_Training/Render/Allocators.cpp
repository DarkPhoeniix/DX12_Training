#include "stdafx.h"
#include "Allocators.h"

void Allocators::Init(ComPtr<ID3D12Device> device)
{
    Make(device, streams, 32, D3D12_COMMAND_LIST_TYPE_DIRECT);
    Make(device, computes, 32, D3D12_COMMAND_LIST_TYPE_COMPUTE);
    Make(device, copies, 4, D3D12_COMMAND_LIST_TYPE_COPY);
}

Executor* Allocators::Obtain(D3D12_COMMAND_LIST_TYPE type)
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

    return nullptr;
}

void Allocators::Make(ComPtr<ID3D12Device> device, std::vector<Executor>& vecExec, unsigned int size, D3D12_COMMAND_LIST_TYPE type)
{
    vecExec.resize(size);

    for (auto& exec : vecExec)
    {
        exec.Allocate(device, type);
        exec.SetFree(true);
    }
}
