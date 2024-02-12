#pragma once

#include "Executor.h"

struct AllocatorPool
{
    void Init(ComPtr<ID3D12Device> device);

    Executor* Obtain(D3D12_COMMAND_LIST_TYPE type);

protected:
    void Make(ComPtr<ID3D12Device> device, std::vector<Executor>& vecExec, unsigned int size, D3D12_COMMAND_LIST_TYPE type);

    std::vector<Executor> streams;
    std::vector<Executor> computes;
    std::vector<Executor> copies;
};
