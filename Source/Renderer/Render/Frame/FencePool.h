#pragma once

#include "RHI/Fence.h"

class FencePool
{
public:
    void Init();

    dx12::Fence* Obtain();

private:
    std::vector<dx12::Fence> fences;
};
