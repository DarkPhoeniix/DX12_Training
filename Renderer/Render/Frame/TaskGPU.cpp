#include "RendererPCH.h"

#include "TaskGPU.h"

#include "CommandList.h"

#include "IGPUCrashTracker.h"
#include "ICommandListCrashContext.h"

TaskGPU::TaskGPU()
    : _commandQueue(nullptr)
    , _fence(nullptr)
    , _commandListCrashContext(dx12::Device::GetCrashTracker()->CreateCommandListCrashContext())
{
}

TaskGPU::~TaskGPU()
{
    _commandQueue = nullptr;
    _fence = nullptr;
}

void TaskGPU::SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue)
{
    _commandQueue = commandQueue;
}

ComPtr<ID3D12CommandQueue> TaskGPU::GetCommandQueue() const
{
    return _commandQueue;
}

void TaskGPU::AddCommandList(dx12::CommandList* commandList)
{
    _commandLists.push_back(commandList);
    _commandListCrashContext->Initialize(commandList->GetDXCommandList().Get());
}

std::vector<dx12::CommandList*> TaskGPU::GetCommandLists() const
{
    return _commandLists;
}

void TaskGPU::SetFence(dx12::Fence* fence)
{
    _fence = fence;
}

dx12::Fence* TaskGPU::GetFence() const
{
    return _fence;
}

ID3D12Fence* TaskGPU::GetDXFence() const
{
    return _fence->GetDXFence().Get();
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

void TaskGPU::SetName(const std::string& name)
{
    _name = name;
}

const std::string& TaskGPU::GetName() const
{
    return _name;
}

std::shared_ptr<tracking::ICommandListCrashContext> TaskGPU::GetCrashContext()
{
    return _commandListCrashContext;
}
