#include "RendererPCH.h"

#include "Frame.h"

#include "Fence.h"

Frame::Frame(rhi::Device* device)
    : Index(0)
    , Prev(nullptr)
    , Next(nullptr)
    , _currentTasks{}
    , _tasks{}
    , _allocatorPool(nullptr)
    , _fencePool(nullptr)
    , _syncPoint(nullptr)
    , _targetTexture(nullptr)
    , _frameBuffer(nullptr)
    , _device(device)
{
}

Frame::~Frame()
{
    Prev = nullptr;
    Next = nullptr;

    _allocatorPool = nullptr;
    _fencePool = nullptr;
    _syncPoint = nullptr;
}

void Frame::Init(std::uint32_t width, std::uint32_t height)
{
    // Create resource for the target texture
    {
        rhi::ClearValue targetClearValue =
        {
            .Format = rhi::Format::R8G8B8A8_UNORM,
            .Color = { 0.0f, 0.0f, 0.0f, 1.0f }
        };

        rhi::TextureDescription textureDesc =
        {
            .Width = width,
            .Height = height,
            .ClearValue = targetClearValue,
            .Format = rhi::Format::R8G8B8A8_UNORM,
            .Dimension = rhi::TextureDimension::Texture2D,
            .Flags = rhi::ResourceFlags::AllowRenderTarget | rhi::ResourceFlags::AllowUnorderedAccess
        };

        _targetTexture = _device->CreateTexture(textureDesc, rhi::ResourceState::Common, std::format("Frame {}", Index));
    }

    // TODO: refactor this
    _tasks.reserve(256);
}

TaskGPU* Frame::CreateTask(rhi::CommandListType type, rhi::PipelineState* rootSignature)
{
    Executor* executor = _allocatorPool->Obtain(type);
    ASSERT(executor, "Failed to obtain executor from allocator pool.");
    _currentTasks.push_back(executor);

    executor->Reset(rootSignature);
    executor->SetFree(false);

    _tasks.push_back(TaskGPU(_device));
    TaskGPU* task = &_tasks.back();

    task->AddCommandList(executor->GetCommandList());

    rhi::Fence* taskFence = _fencePool->Obtain();
    ASSERT(taskFence, "Failed to obtain fence from fence pool.");
    task->SetFence(taskFence);
    taskFence->SetFree(false);
    taskFence->SetValue(taskFence->GetValue() + 1);

    return task;
}

void Frame::WaitCPU()
{
    if (_syncPoint)
    {
        _syncPoint->Wait();
        _syncPoint->SetFree(true);
        _syncPoint = nullptr;
    }
    else
    {
        LOG_WARNING("No sync point set for the frame. Skipping CPU wait.");
    }
}

void Frame::ResetGPU()
{
    for (auto& task : _currentTasks)
    {
        task->SetFree(true);
    }

    for (auto& task : _tasks)
    {
        task.GetFence()->SetFree(true);
    }

    _tasks.clear();
    _currentTasks.clear();
}

void Frame::Resize(std::uint32_t width, std::uint32_t height)
{
    //_targetTexture->Reset();

    Init(width, height);
}

void Frame::SetAllocatorPool(AllocatorPool* allocatorPool)
{
    ASSERT(allocatorPool, "Allocator pool is nullptr when trying to set it in the frame.");
    _allocatorPool = allocatorPool;
}

void Frame::SetFencePool(FencePool* fencePool)
{
    ASSERT(fencePool, "Fence pool is nullptr when trying to set it in the frame.");
    _fencePool = fencePool;
}

TaskGPU* Frame::GetTask(const std::string& name)
{
    for (auto& task : _tasks)
    {
        if (task.GetName() == name)
        {
            return &task;
        }
    }

    LOG_WARNING("Task with name '{}' not found in the frame.", name);
    return nullptr;
}

std::vector<TaskGPU> Frame::GetTasks() const
{
    return _tasks;
}

std::shared_ptr<rhi::Texture> Frame::GetTargetTexture()
{
    return _targetTexture;
}

void Frame::SetBuffer(std::shared_ptr<rhi::Buffer> buffer)
{
    _frameBuffer = buffer;
}

std::shared_ptr<rhi::Buffer> Frame::GetBuffer() const
{
    return _frameBuffer;
}

void Frame::SetSyncPoint(rhi::Fence* syncPoint)
{
    _syncPoint = syncPoint;
}

rhi::Fence* Frame::GetSyncPoint() const
{
    return _syncPoint;
}
