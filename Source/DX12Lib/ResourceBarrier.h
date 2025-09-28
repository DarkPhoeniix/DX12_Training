#pragma once

#include "Resource.h"

namespace dx12
{
    class CommandList;

    class ResourceBarrier
    {
    public:
        std::weak_ptr<Resource> TargetResource;
        D3D12_RESOURCE_STATES BeforeState;
        D3D12_RESOURCE_STATES AfterState;

        ResourceBarrier(std::shared_ptr<Resource> targetResource = nullptr, D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATES afterState = D3D12_RESOURCE_STATE_COMMON);

        void Transition(CommandList& commandList);
    };
} // namespace dx12
