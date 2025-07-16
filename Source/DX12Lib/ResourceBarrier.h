#pragma once

#include "Resource.h"

namespace dx12
{
    class CommandList;

    class ResourceBarrier
    {
    public:
        std::weak_ptr<Resource> Resource;
        D3D12_RESOURCE_STATES BeforeState;
        D3D12_RESOURCE_STATES AfterState;

        void Transition(CommandList& commandList);
    };
} // namespace dx12
