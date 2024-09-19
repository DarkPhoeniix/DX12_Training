#pragma once

#include "DXObjects/SwapChain.h"
#include "Render/AllocatorPool.h"
#include "Render/Executor.h"
#include "Render/TaskGPU.h"
#include "Render/FencePool.h"
#include "DXObjects/DescriptorHeap.h"

// TODO: refactor the Frame class

namespace Core
{
    class RootSignature;
} // namespace Core

class Frame
{
public:
    Frame();
    ~Frame();

    void Init(const Core::SwapChain& swapChain);

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, Core::RootSignature* rootSignature = nullptr);

    void WaitCPU();
    void ResetGPU();

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);

    void SetSyncFrame(Core::Fence* syncFrame);
    Core::Fence* GetSyncFrame() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

    Core::Resource _swapChainTexture;
    Core::Resource _targetTexture;
    Core::Resource _depthTexture;

    ComPtr<ID3D12DescriptorHeap> _targetHeap;
    ComPtr<ID3D12DescriptorHeap> _depthHeap;

    Core::DescriptorHeap _postFXDescHeap;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<Executor*> _executedTasks;

    ID3D12CommandQueue* _queueStream;
    ID3D12CommandQueue* _queueCompute;
    ID3D12CommandQueue* _queueCopy;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    Core::Fence* _syncFrame;

    std::vector<TaskGPU> _tasks;
};
