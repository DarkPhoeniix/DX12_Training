#pragma once

#include "Renderer/Render/Frame/AllocatorPool.h"
#include "Renderer/Render/Frame/Executor.h"
#include "Renderer/Render/Frame/TaskGPU.h"
#include "Renderer/Render/Frame/FencePool.h"

#include "RenderGraph/Interfaces.h"

// TODO: refactor the Frame class

namespace dx12
{
    class PipelineState;
} // namespace core

class Frame : public rg::ITaskAllocator
{
public:
    Frame(rhi::Device* device);
    Frame(const Frame& other) = default;
    Frame(Frame&& other) = default;
    ~Frame();

    Frame& operator=(const Frame& other) = default;
    Frame& operator=(Frame&& other) = default;

    void Init(std::uint32_t width, std::uint32_t height);

    rg::ITask* AllocateTask(rhi::CommandListType type, rhi::PipelineState* rootSignature) override;

    void WaitCPU();
    void ResetGPU();

    void Resize(std::uint32_t width, std::uint32_t height);

    void SetFencePool(FencePool* fencePool);

    void SetSyncPoint(rhi::Fence* syncPoint);
    rhi::Fence* GetSyncPoint() const;

    TaskGPU* GetTask(const std::string& name);
    std::vector<std::unique_ptr<TaskGPU>>& GetTasks();

    std::shared_ptr<rhi::Texture> GetTargetTexture();

    void SetBuffer(std::shared_ptr<rhi::Buffer> buffer);
    std::shared_ptr<rhi::Buffer> GetBuffer() const;

    unsigned int Index;
    Frame* Prev;
    Frame* Next;

private:
    std::vector<std::unique_ptr<TaskGPU>> _tasks;

    FencePool* _fencePool;
    rhi::Fence* _syncPoint;

    std::shared_ptr<rhi::Texture> _targetTexture;
    std::shared_ptr<rhi::Buffer> _frameBuffer;

    rhi::Device* _device;
};
