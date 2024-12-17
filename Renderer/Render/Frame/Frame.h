#pragma once

#include "SwapChain.h"
#include "Render/Frame/AllocatorPool.h"
#include "Render/Frame/Executor.h"
#include "Render/Frame/TaskGPU.h"
#include "Render/Frame/FencePool.h"
#include "DescriptorHeap.h"

// TODO: refactor the Frame class

namespace dx12
{
    class RootSignature;
} // namespace Core

class Frame
{
public:
    Frame();
    ~Frame();

    void Init(const DirectX::XMUINT2& size);

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, dx12::RootSignature* rootSignature = nullptr);

    void WaitCPU();
    void ResetGPU();

    void Resize(const DirectX::XMUINT2& size);

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);

    void SetSyncFrame(dx12::Fence* syncFrame);
    dx12::Fence* GetSyncFrame() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    void SetSwapChainTexture(dx12::Resource* texture);

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

    dx12::Resource* _swapChainTexture;
    dx12::Resource _targetTexture;
    dx12::Resource _depthTexture;

    ComPtr<ID3D12DescriptorHeap> _targetHeap;
    ComPtr<ID3D12DescriptorHeap> _depthHeap;
    dx12::DescriptorHeap _testHeap;

    dx12::DescriptorHeap _postFXDescHeap;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<Executor*> _executedTasks;

    ID3D12CommandQueue* _queueStream;
    ID3D12CommandQueue* _queueCompute;
    ID3D12CommandQueue* _queueCopy;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    dx12::Fence* _syncFrame;

    std::vector<TaskGPU> _tasks;
};
