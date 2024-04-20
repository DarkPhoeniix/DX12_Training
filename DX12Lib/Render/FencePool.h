#pragma once

#include "DXObjects/Fence.h"

class FencePool
{
public:
    void Init();

    Core::Fence* Obtain();

private:
    std::vector<Core::Fence> fences;
};
