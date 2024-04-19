#pragma once

#include "DXObjects/Fence.h"

namespace Core
{
    class GraphicsCommandList;
} // namespace Core

class TaskGPU
{
public:
    TaskGPU();
    ~TaskGPU();

    void AddCommandList(Core::GraphicsCommandList* commandList);
    std::vector<Core::GraphicsCommandList*> GetCommandLists() const;

    void SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue);
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

    void SetFence(Core::Fence* fence);
    Core::Fence* GetFence() const;
    ID3D12Fence* GetDXFence() const;
    UINT64 GetFenceValue() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

private:
    std::vector<Core::GraphicsCommandList*> _commandLists;
    ComPtr<ID3D12CommandQueue> _commandQueue;

    Core::Fence* _fence = nullptr;
    std::vector<std::string> _dependencies;

    std::string _name;
};

