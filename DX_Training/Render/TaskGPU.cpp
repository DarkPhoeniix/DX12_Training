#include "stdafx.h"
#include "TaskGPU.h"

void TaskGPU::SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue)
{
    _commandQueue = commandQueue;
}

ComPtr<ID3D12CommandQueue> TaskGPU::GetCommandQueue() const
{
    return _commandQueue;
}

void TaskGPU::AddCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    _commandLists.push_back(commandList);
}

std::vector<ComPtr<ID3D12GraphicsCommandList2>> TaskGPU::GetCommandLists() const
{
    return _commandLists;
}

void TaskGPU::SetFence(Fence* fence)
{
    _fence = fence;
}

Fence* TaskGPU::GetFence() const
{
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

void TaskGPU::SetName(const std::string& name)
{
    _name = name;
}

const std::string& TaskGPU::GetName() const
{
    return _name;
}
