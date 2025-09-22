#include "DX12LibPCH.h"

#include "ResourceBarrier.h"

#include "CommandList.h"

namespace dx12
{
    ResourceBarrier::ResourceBarrier(std::shared_ptr<Resource> targetResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
        : TargetResource(targetResource)
        , BeforeState(beforeState)
        , AfterState(afterState)
    {
    }

    void ResourceBarrier::Transition(CommandList& commandList)
    {
        commandList.TransitionBarrier(*this);
    }
} // namespace dx12
