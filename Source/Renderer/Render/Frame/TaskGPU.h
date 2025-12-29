#pragma once

#include "GPUCrashTracker/ICommandListCrashContext.h"

namespace dx12
{
    class CommandList;
    class Fence;
} // namespace dx12

class TaskGPU
{
public:
    TaskGPU();
    ~TaskGPU();

    void AddCommandList(dx12::CommandList* commandList);
    std::vector<dx12::CommandList*> GetCommandLists() const;
    dx12::CommandList& GetCommandList();

    void SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue);
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

    void SetFence(dx12::Fence* fence);
    dx12::Fence* GetFence() const;
    ID3D12Fence* GetDXFence() const;
    UINT64 GetFenceValue() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

    std::shared_ptr<tracking::ICommandListCrashContext> GetCrashContext();

private:
    std::vector<dx12::CommandList*> _commandLists;
    ComPtr<ID3D12CommandQueue> _commandQueue;
    std::shared_ptr<tracking::ICommandListCrashContext> _commandListCrashContext;

    dx12::Fence* _fence = nullptr;
    std::vector<std::string> _dependencies;

    std::string _name;
};

