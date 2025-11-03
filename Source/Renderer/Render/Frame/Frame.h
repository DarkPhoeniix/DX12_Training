#pragma once

#include "Renderer/Render/Frame/AllocatorPool.h"
#include "Renderer/Render/Frame/Executor.h"
#include "Renderer/Render/Frame/TaskGPU.h"
#include "Renderer/Render/Frame/FencePool.h"

// TODO: refactor the Frame class

namespace dx12
{
    class PipelineState;
} // namespace core

class Frame
{
public:
    Frame();
    ~Frame();

    void Init(const DirectX::XMUINT2& size);

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, dx12::PipelineState* rootSignature = nullptr);

    void WaitCPU();
    void ResetGPU();

    void Resize(const DirectX::XMUINT2& size);

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);

    void SetSyncPoint(dx12::Fence* syncPoint);
    dx12::Fence* GetSyncPoint() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    std::shared_ptr<dx12::Resource> GetTargetTexture();

    void SetBuffer(std::shared_ptr<dx12::Resource> buffer);
    std::shared_ptr<dx12::Resource> GetBuffer() const;

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<TaskGPU> _tasks;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    dx12::Fence* _syncPoint;

    std::shared_ptr<dx12::Resource> _targetTexture;
    std::shared_ptr<dx12::Resource> _frameBuffer;
};
