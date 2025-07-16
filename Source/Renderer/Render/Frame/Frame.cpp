#include "RendererPCH.h"

#include "Frame.h"

#include "CommandList.h"
#include "Fence.h"
#include "SwapChain.h"
#include "PipelineState.h"

Frame::Frame()
    : Index(0)
    , Prev(nullptr)
    , Next(nullptr)
    , _targetTexture(nullptr)
    , _currentTasks{}
    , _allocatorPool(nullptr)
    , _fencePool(nullptr)
    , _syncPoint(nullptr)
    , _tasks{}
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

void Frame::Init(const DirectX::XMUINT2& size, uint32_t cacheSize)
{
    {
        // TODO: that sucks too
        _resourceTable.Init(1024, true);
    }

    // Initialize cache heap
    {
        dx12::ResourceDescription desc = {};
        desc.SetSize({ _16MB, 1 });
        desc.SetStride(256);
        desc.SetFormat(DXGI_FORMAT_UNKNOWN);
        desc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);

        std::shared_ptr<dx12::Resource> frameCachedMemory = ResourceFactory::Create(std::format("Frame cache {}", Index), desc);
        frameCachedMemory->CreateCommitedResource();

        _cache.SetResource(frameCachedMemory);
    }

    // Create resource for the target texture
    {
        D3D12_CLEAR_VALUE clearValueTexTarget;
        {
            clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            clearValueTexTarget.Color[0] = 0.0f;
            clearValueTexTarget.Color[1] = 0.0f;
            clearValueTexTarget.Color[2] = 0.0f;
            clearValueTexTarget.Color[3] = 1.0f;
        }

        dx12::ResourceDescription textureDesc;
        {
            textureDesc.SetSize(size);
            textureDesc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
            textureDesc.SetLayout(D3D12_TEXTURE_LAYOUT_UNKNOWN);
            textureDesc.SetMipLevels(1);
            textureDesc.SetAlignment(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
            textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            textureDesc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            textureDesc.SetClearValue(clearValueTexTarget);
            textureDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::RenderTarget);
        }
        _targetTexture = ResourceFactory::Create(std::format("Frame cache {}", Index), textureDesc);
        _targetTexture->CreateCommitedResource();

        _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::RTV);
        _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::SRV);
        _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::UAV);
    }

    // TODO: refactor this
    _tasks.reserve(128);
}

TaskGPU* Frame::CreateTask(D3D12_COMMAND_LIST_TYPE type, dx12::PipelineState* rootSignature)
{
    Executor* executor = _allocatorPool->Obtain(type);
    ASSERT(executor, "Failed to obtain executor from allocator pool.");
    _currentTasks.push_back(executor);

    executor->Reset(rootSignature);
    executor->SetFree(false);

    _tasks.push_back({});
    TaskGPU* task = &_tasks.back();
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        task->SetCommandQueue(dx12::Device::GetStreamQueue());
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        task->SetCommandQueue(dx12::Device::GetComputeQueue());
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        task->SetCommandQueue(dx12::Device::GetCopyQueue());
        break;
    default:
        FAIL(false, "Unsupported command list type.");
        return nullptr;
    }

    task->AddCommandList(executor->GetCommandList());

    dx12::Fence* taskFence = _fencePool->Obtain();
    ASSERT(taskFence, "Failed to obtain fence from fence pool.");
    task->SetFence(taskFence);
    taskFence->SetFree(false);
    taskFence->SetValue(taskFence->GetValue() + 1);

    return task;
}

void Frame::BindDescriptorHeaps(dx12::CommandList& commandList)
{
    commandList.SetDescriptorHeaps({ _resourceTable.GetDescriptorHeap(dx12::ResourceViewType::SRV).GetDXDescriptorHeap().Get()});
}

dx12::ResourceTable& Frame::GetResourceTable()
{
    return _resourceTable;
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

CacheGPU& Frame::GetCache()
{
    return _cache;
}

void Frame::ResetCache()
{
    _resourceTable.Reset();
    _cache.Clear();

    _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::RTV);
    _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::SRV);
    _resourceTable.PlaceResource(_targetTexture, dx12::ResourceViewType::UAV);
}

void Frame::Resize(const DirectX::XMUINT2& size)
{
    _targetTexture->Reset();
    _resourceTable.Reset();
    _cache.Clear();

    Init(size);
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

std::shared_ptr<dx12::Resource> Frame::GetTargetTexture()
{
    return _targetTexture;
}

void Frame::SetSyncPoint(dx12::Fence* syncPoint)
{
    _syncPoint = syncPoint;
}

dx12::Fence* Frame::GetSyncPoint() const
{
    return _syncPoint;
}
