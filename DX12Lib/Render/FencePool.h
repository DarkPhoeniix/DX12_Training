#pragma once

#include "Fence.h"

class FencePool
{
public:
    void Init(ComPtr<ID3D12Device> device);

    Fence* Obtain();

private:
    std::vector<Fence> fences;
};
