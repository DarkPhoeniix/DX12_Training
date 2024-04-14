#pragma once

#include "DXObjects/Fence.h"

class FencePool
{
public:
    void Init(ComPtr<ID3D12Device2> device);

    Fence* Obtain();

private:
    std::vector<Fence> fences;
};
