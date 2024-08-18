#include "stdafx.h"

#include "TaskGPU.h"

#include "DXObjects/GraphicsCommandList.h"

TaskGPU::TaskGPU()
    : _commandQueue(nullptr)
    , _fence(nullptr)
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

void TaskGPU::AddCommandList(Core::GraphicsCommandList* commandList)
{
    _commandLists.push_back(commandList);
}

std::vector<Core::GraphicsCommandList*> TaskGPU::GetCommandLists() const
{
    return _commandLists;
}

void TaskGPU::SetFence(Core::Fence* fence)
{
    _fence = fence;
}

Core::Fence* TaskGPU::GetFence() const
{
    return _fence;
}

ID3D12Fence* TaskGPU::GetDXFence() const
{
    return _fence->GetFence().Get();
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
