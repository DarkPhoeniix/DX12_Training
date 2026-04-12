#include "RendererPCH.h"

#include "TaskGPU.h"

#include "Core/DescriptorHeapManager.h"

#include "GPUCrashTracker/IGPUCrashTracker.h"
#include "GPUCrashTracker/ICommandListCrashContext.h"

#include "RHI/CommandList.h"
#include "RHI/Fence.h"

TaskGPU::TaskGPU(rhi::Device* device, rhi::CommandListType type)
    : _fence(nullptr)
    , _commandList(device->CreateCommandList(type))
    , _type(type)
    //, _commandListCrashContext(device->GetCrashTracker()->CreateCommandListCrashContext())
{
    _commandList->Close();
}

TaskGPU::~TaskGPU()
{
    return;
}

void TaskGPU::Reset(rhi::PipelineState* pipelineState)
{
    _commandList->Reset(pipelineState);
    _commandList->SetDescriptorHeaps(DescriptorHeapManager::Get().GetShaderResourcesDescriptorHeap());
}

rhi::CommandList* TaskGPU::GetCommandList()
{
    return _commandList.get();
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
    return _type;
}

void TaskGPU::SetName(const std::string& name)
{
    _name = name;

    _commandList->SetName(name + "_command_list");

    // TODO: not the best place to set the marker, but this will definetly register all command lists
    //_commandListCrashContext->SetMarker(name);
}

const std::string& TaskGPU::GetName() const
{
    return _name;
}

tracking::ICommandListCrashContext* TaskGPU::GetCrashContext()
{
    return _commandListCrashContext.get();
}
