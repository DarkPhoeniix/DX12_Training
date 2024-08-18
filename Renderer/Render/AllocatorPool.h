#pragma once

#include "Executor.h"

struct AllocatorPool
{
    void Init();

    Executor* Obtain(D3D12_COMMAND_LIST_TYPE type);

protected:
    void Make(std::vector<Executor>& vecExec, unsigned int size, D3D12_COMMAND_LIST_TYPE type);

    std::vector<Executor> streams;
    std::vector<Executor> computes;
    std::vector<Executor> copies;

    ComPtr<ID3D12Device2> _DXDevice;
};
