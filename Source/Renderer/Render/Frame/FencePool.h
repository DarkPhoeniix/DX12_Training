#pragma once

#include "RHI/Fence.h"

class FencePool
{
public:
    FencePool(rhi::Device* device);

    void Init();

    rhi::Fence* Obtain();

private:
    std::vector<std::unique_ptr<rhi::Fence>> _fences;

    rhi::Device* _device;
};
