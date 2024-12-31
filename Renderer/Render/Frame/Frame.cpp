#include "stdafx.h"

#include "Frame.h"

#include "CommandList.h"
#include "Fence.h"
#include "SwapChain.h"
#include "PipelineState.h"

Frame::Frame()
    : Index(0)
    , Prev(nullptr)
    , Next(nullptr)
    , _targetTexture{}
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

void Frame::Init(const DirectX::XMUINT2& size)
{
    // Create descriptor heaps (RTV / DSR / CBV_SRV_UAV)
    {
        dx12::DescriptorHeapDescription desc = {};
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        desc.SetNumDescriptors(32);
        desc.SetNodeMask(0);

        _RTVHeap.Create(desc);

        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        _DSVHeap.Create(desc);

        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        _BuffersHeap.Create(desc);
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
            textureDesc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::RenderTarget);
        }

        _targetTexture.CreateCommitedResource(textureDesc, D3D12_RESOURCE_STATE_COPY_SOURCE);
        _targetTexture.SetName(std::string("Frame RTT ") + std::to_string(Index));
    }

    // Create RTV on descriptor heap
    {
        dx12::Device::CreateRenderTargetView(_targetTexture.GetAsRTV(), _RTVHeap);
    }

    {
        dx12::HeapDescription desc = {};
        desc.SetSize(_32MB);
        desc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
        _resourcesHeap.Create(desc);

        _cache.SetHeap(std::make_shared<dx12::Heap>(_resourcesHeap));
    }
}

TaskGPU* Frame::CreateTask(D3D12_COMMAND_LIST_TYPE type, dx12::PipelineState* rootSignature)
{
    Executor* executor = _allocatorPool->Obtain(type);
    _currentTasks.push_back(executor);

    executor->Reset(rootSignature);
    executor->SetFree(false);

    _tasks.push_back({});
    TaskGPU* task = &_tasks.back();
    if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        task->SetCommandQueue(dx12::Device::GetStreamQueue());
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        task->SetCommandQueue(dx12::Device::GetComputeQueue());
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
    {
        task->SetCommandQueue(dx12::Device::GetCopyQueue());
    }

    task->AddCommandList(executor->GetCommandList());

    dx12::Fence* taskFence = _fencePool->Obtain();
    task->SetFence(taskFence);
    taskFence->SetFree(false);
    taskFence->SetValue(taskFence->GetValue() + 1);

    return task;
}

void Frame::BindDescriptorHeaps(dx12::CommandList& commandList)
{
    commandList.SetDescriptorHeaps({ _BuffersHeap.GetDXDescriptorHeap().Get() });
}

dx12::DescriptorHeap& Frame::GetDescriptorHeap(dx12::DescriptorHeapType type)
{
    switch (type)
    {
    case dx12::DescriptorHeapType::RTV:
        return _RTVHeap;
    case dx12::DescriptorHeapType::DSV:
        return _DSVHeap;
    case dx12::DescriptorHeapType::CBV_SRV_UAV:
        return _BuffersHeap;
    }
}

void Frame::WaitCPU()
{
    if (_syncPoint)
    {
        _syncPoint->Wait();
        _syncPoint->SetFree(true);
        _syncPoint = nullptr;
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
    _RTVHeap.Reset();
    _DSVHeap.Reset();
    _BuffersHeap.Reset();

    _cache.Reset();

    dx12::Device::CreateRenderTargetView(_targetTexture.GetAsRTV(), _RTVHeap);
}

void Frame::Resize(const DirectX::XMUINT2& size)
{
    _targetTexture.Reset();

    _RTVHeap.Reset();
    _DSVHeap.Reset();
    _BuffersHeap.Reset();

    Init(size);
}

void Frame::SetAllocatorPool(AllocatorPool* allocatorPool)
{
    _allocatorPool = allocatorPool;
}

void Frame::SetFencePool(FencePool* fencePool)
{
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

    return nullptr;
}

std::vector<TaskGPU> Frame::GetTasks() const
{
    return _tasks;
}

dx12::Resource& Frame::GetTargetTexture()
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
