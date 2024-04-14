#pragma once

#include "Render/AllocatorPool.h"
#include "Render/Executor.h"
#include "Render/TaskGPU.h"
#include "Render/FencePool.h"

// TODO: refactor the Frame class

class SwapChain;

class Frame
{
public:
    Frame();
    ~Frame();

    void Init(const SwapChain& swapChain);

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12PipelineState> pipelineState);

    void WaitCPU();
    void ResetGPU();

    void SetDirectQueue(ID3D12CommandQueue* directQueue);
    ID3D12CommandQueue* GetDirectQueue() const;

    void SetComputeQueue(ID3D12CommandQueue* computeQueue);
    ID3D12CommandQueue* GetComputeQueue() const;

    void SetCopyQueue(ID3D12CommandQueue* copyQueue);
    ID3D12CommandQueue* GetCopyQueue() const;

    void SetDXDevice(ComPtr<ID3D12Device2> device);

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);
    void SetDXDevice(ID3D12Device2* DXDevice);

    void SetSyncFrame(Fence* syncFrame);
    Fence* GetSyncFrame() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

    Resource _swapChainTexture;
    Resource _targetTexture;
    Resource _depthTexture;

    ComPtr<ID3D12DescriptorHeap> _targetHeap;
    ComPtr<ID3D12DescriptorHeap> _depthHeap;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<Executor*> _executedTasks;

    ID3D12CommandQueue* _queueStream;
    ID3D12CommandQueue* _queueCompute;
    ID3D12CommandQueue* _queueCopy;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    Fence* _syncFrame;

    std::vector<TaskGPU> _tasks;

    ComPtr<ID3D12Device2> _DXDevice;
};
