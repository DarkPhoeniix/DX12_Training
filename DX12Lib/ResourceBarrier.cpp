#include "DX12LibPCH.h"

#include "ResourceBarrier.h"

#include "CommandList.h"

void dx12::ResourceBarrier::Transition(CommandList& commandList)
{
    commandList.TransitionBarrier(*this);
}
