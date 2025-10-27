#pragma once

#include "Resource.h"

namespace dx12
{
    class CommandList;

    class ResourceBarrier
    {
    public:
        std::weak_ptr<Resource> TargetResource;
        ResourceState BeforeState;
        ResourceState AfterState;

        ResourceBarrier(std::shared_ptr<Resource> targetResource = nullptr, ResourceState beforeState = ResourceState::Common, ResourceState afterState = ResourceState::Common);

        void Transition(CommandList& commandList);
    };
} // namespace dx12
