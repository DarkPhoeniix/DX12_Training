#include "RendererPCH.h"

#include "TaskGPU.h"

#include "RHI/CommandList.h"
#include "RHI/Fence.h"

#include "GPUCrashTracker/IGPUCrashTracker.h"
#include "GPUCrashTracker/ICommandListCrashContext.h"

TaskGPU::TaskGPU(rhi::Device* device)
    : _fence(nullptr)
    , _commandListCrashContext(device->GetCrashTracker()->CreateCommandListCrashContext())
{
}

TaskGPU::~TaskGPU()
{
    _fence = nullptr;
}

void TaskGPU::AddCommandList(rhi::CommandList* commandList)
{
    ASSERT(commandList, "Trying to add a nullptr command list to the task.");

    _commandLists.push_back(commandList);
    _commandListCrashContext->Initialize(commandList);
}

rhi::CommandList* TaskGPU::GetCommandList()
{
    return _commandLists.front();
}

void TaskGPU::SetFence(rhi::Fence* fence)
{
    ASSERT(fence, "Trying to set a nullptr fence to the task.");
    _fence = fence;
}

rhi::Fence* TaskGPU::GetFence() const
{
    ASSERT(_fence, "Trying to get a nullptr fence from the task.");
    return _fence;
}

UINT64 TaskGPU::GetFenceValue() const
{
    return _fence->GetValue();
}

void TaskGPU::AddDependency(const std::string& taskName)
{
    _dependencies.push_back(taskName);
}

std::vector<std::string> TaskGPU::GetDependencies() const
{
    return _dependencies;
}

rhi::CommandListType TaskGPU::GetType() const
{
    return _commandLists.front()->GetCommandListType();
}

void TaskGPU::SetName(const std::string& name)
{
    _name = name;

    // TODO: not the best place to set the marker, but this will definetly register all command lists
    _commandListCrashContext->SetMarker(name);
}

const std::string& TaskGPU::GetName() const
{
    return _name;
}

std::shared_ptr<tracking::ICommandListCrashContext> TaskGPU::GetCrashContext()
{
    return _commandListCrashContext;
}
