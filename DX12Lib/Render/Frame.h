#pragma once

#include "AllocatorPool.h"
#include "Executor.h"
#include "TaskGPU.h"
#include "FencePool.h"

class Frame
{
public:
    Frame() = default;
    ~Frame();

    void Init(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain> swapChain);

    void SetDirectQueue(ComPtr<ID3D12CommandQueue> directQueue);
    ComPtr<ID3D12CommandQueue> GetDirectQueue() const;

    void SetComputeQueue(ComPtr<ID3D12CommandQueue> computeQueue);
    ComPtr<ID3D12CommandQueue> GetComputeQueue() const;

    void SetCopyQueue(ComPtr<ID3D12CommandQueue> copyQueue);
    ComPtr<ID3D12CommandQueue> GetCopyQueue() const;

    void SetAllocatorPool(AllocatorPool* allocatorPool);

    void SetFencePool(FencePool* fencePool);

    void SetSyncFrame(Fence* syncFrame);
    Fence* GetSyncFrame() const;

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12PipelineState> pipelineState);

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    void WaitCPU();
    void ResetGPU();

    unsigned int Index = 0;
    Frame* Prev = nullptr;
    Frame* Next = nullptr;

    Resource _swapChainTexture;
    Resource _targetTexture;
    Resource _depthTexture;

    ComPtr<ID3D12DescriptorHeap> _targetHeap;
    ComPtr<ID3D12DescriptorHeap> _depthHeap;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<Executor*> _executedTasks;

    ComPtr<ID3D12CommandQueue> _queueStream;
    ComPtr<ID3D12CommandQueue> _queueCompute;
    ComPtr<ID3D12CommandQueue> _queueCopy;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    Fence* _syncFrame = nullptr;

    std::vector<TaskGPU> _tasks;
};
