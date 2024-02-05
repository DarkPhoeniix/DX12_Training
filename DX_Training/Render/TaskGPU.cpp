#include "stdafx.h"
#include "TaskGPU.h"

TaskGPU::TaskGPU(const std::string& name)
    : _name(name)
{   }

void TaskGPU::SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue)
{
    _commandQueue = commandQueue;
}

void TaskGPU::AddCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    _commandLists.push_back(commandList);
}

std::vector<ComPtr<ID3D12GraphicsCommandList2>> TaskGPU::GetCommandLists() const
{
    return _commandLists;
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
