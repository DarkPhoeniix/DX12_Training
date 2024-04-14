#pragma once

#include "DXObjects/Fence.h"

class TaskGPU
{
public:
    TaskGPU();
    ~TaskGPU();

    void AddCommandList(ComPtr<ID3D12GraphicsCommandList2>);
    std::vector<ComPtr<ID3D12GraphicsCommandList2>> GetCommandLists() const;

    void SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue);
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const;

    void SetFence(Fence* fence);
    Fence* GetFence() const;
    ID3D12Fence* GetDXFence() const;
    UINT64 GetFenceValue() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

private:
    std::vector<ComPtr<ID3D12GraphicsCommandList2>> _commandLists;
    ComPtr<ID3D12CommandQueue> _commandQueue;

    Fence* _fence = nullptr;
    std::vector<std::string> _dependencies;

    std::string _name;
};

