#pragma once

#include "Executor.h"

struct AllocatorPool
{
    void Init(rhi::Device* device);

    Executor* Obtain(rhi::CommandListType type);

protected:
    void Make(rhi::Device* device, std::vector<Executor>& vecExec, unsigned int size, rhi::CommandListType type);

    std::vector<Executor> streams;
    std::vector<Executor> computes;
    std::vector<Executor> copies;
};
