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
    Frame(rhi::Device* device);
    ~Frame();

    void Init(std::uint32_t width, std::uint32_t height);

    TaskGPU* CreateTask(rhi::CommandListType type, rhi::PipelineState* rootSignature = nullptr);

    void WaitCPU();
    void ResetGPU();

    void Resize(std::uint32_t width, std::uint32_t height);

    void SetAllocatorPool(AllocatorPool* allocatorPool);
    void SetFencePool(FencePool* fencePool);

    void SetSyncPoint(rhi::Fence* syncPoint);
    rhi::Fence* GetSyncPoint() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<TaskGPU> GetTasks() const;

    std::shared_ptr<rhi::Texture> GetTargetTexture();

    void SetBuffer(std::shared_ptr<rhi::Buffer> buffer);
    std::shared_ptr<rhi::Buffer> GetBuffer() const;

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

private:
    std::vector<Executor*> _currentTasks;
    std::vector<TaskGPU> _tasks;

    AllocatorPool* _allocatorPool;
    FencePool* _fencePool;
    rhi::Fence* _syncPoint;

    std::shared_ptr<rhi::Texture> _targetTexture;
    std::shared_ptr<rhi::Buffer> _frameBuffer;

    rhi::Device* _device;
};
