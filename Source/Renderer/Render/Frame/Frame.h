#pragma once

#include "ResourceTable.h"
#include "Heap.h"

#include "Render/Frame/AllocatorPool.h"
#include "Render/Frame/Executor.h"
#include "Render/Frame/TaskGPU.h"
#include "Render/Frame/FencePool.h"
#include "Render/Frame/CacheGPU.h"

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

    void Init(const DirectX::XMUINT2& size, uint32_t cacheSize = _16MB);

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, dx12::PipelineState* rootSignature = nullptr);

    void BindDescriptorHeaps(dx12::CommandList& commandList);

    void WaitCPU();
    void ResetGPU();

    dx12::ResourceTable& GetResourceTable();

    CacheGPU& GetCache();
    void ResetCache();

    void Resize(const DirectX::XMUINT2& size);

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);

    void SetSyncPoint(dx12::Fence* syncPoint);
    dx12::Fence* GetSyncPoint() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    std::shared_ptr<dx12::Resource> GetTargetTexture();

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

private:
    std::vector<Executor*> _currentTasks;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    dx12::Fence* _syncPoint;

    dx12::Heap _resourcesHeap;

    dx12::ResourceTable _resourceTable;
    CacheGPU _cache;

    std::shared_ptr<dx12::Resource> _targetTexture;

    std::vector<TaskGPU> _tasks;
};
