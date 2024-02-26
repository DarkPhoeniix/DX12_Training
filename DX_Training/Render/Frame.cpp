#include "stdafx.h"
#include "Frame.h"

#include "Fence.h"

TaskGPU* Frame::CreateTask(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12PipelineState> pipelineState)
{
    Executor* exec = _allocatorPool->Obtain(type);
    _currentTasks.push_back(exec);

    exec->Reset(pipelineState);
    exec->SetFree(false);

    _tasks.push_back({});
    TaskGPU* task = &_tasks.back();
    if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
    {
        task->SetCommandQueue(_queueStream);
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
    {
        task->SetCommandQueue(_queueCompute);
    }
    else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
    {
        task->SetCommandQueue(_queueCopy);
    }

    task->AddCommandList(exec->GetCommandList().Get());

    Fence* taskFence = _fencePool->Obtain();
    task->SetFence(taskFence);
    taskFence->SetFree(false);
    taskFence->SetValue(taskFence->GetValue() + 1);

    return task;
}

void Frame::WaitCPU()
{
    if (_syncFrame)
    {
        _syncFrame->Wait();
        _syncFrame->SetFree(true);
        _syncFrame = nullptr;
    }
}

void Frame::ResetGPU()
{
    for (auto& task : _executedTasks)
    {
        task->SetFree(true);
    }

    for (auto& task : _tasks)
    {
        task.GetFence()->SetFree(true);
    }

    _tasks.clear();
    _executedTasks.clear();
    _executedTasks = std::move(_currentTasks);
}

void Frame::Init(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain> swapChain)
{
    ComPtr<ID3D12Resource> temp;
    swapChain->GetBuffer(Index, IID_PPV_ARGS(&temp));

    _swapChainTexture.SetResource(temp);

    ResourceDescription desc = _swapChainTexture.GetResourceDescription();

    // Create resource for the target texture
    {
        D3D12_CLEAR_VALUE clearValueTexTarget;
        clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValueTexTarget.Color[0] = 0.0f;
        clearValueTexTarget.Color[1] = 0.0f;
        clearValueTexTarget.Color[2] = 0.0f;
        clearValueTexTarget.Color[3] = 1.0f;

        desc.AddFlags(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        desc.SetClearValue(clearValueTexTarget);

        _targetTexture = Resource(device, desc);
        _targetTexture.CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_SOURCE);
    }

    // Create resource for the depth texture
    {
        desc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        desc.SetFormat(DXGI_FORMAT_D32_FLOAT);

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth = 1;
        clearValue.DepthStencil.Stencil = 0;

        desc.SetClearValue(clearValue);

        _depthTexture = Resource(device, desc);
        _depthTexture.CreateCommitedResource(D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    // Create descriptor heaps
    {
        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_targetHeap));

        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_depthHeap));
    }

    {
        D3D12_RENDER_TARGET_VIEW_DESC renderTargetDesc = {};
        renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        renderTargetDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargetDesc.Texture2D.MipSlice = 0;
        device->CreateRenderTargetView(_targetTexture.GetResource().Get(), &renderTargetDesc, _targetHeap->GetCPUDescriptorHandleForHeapStart());
    }

    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;//D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Texture2D.MipSlice = 0;
        device->CreateDepthStencilView(_depthTexture.GetResource().Get(), &depthStencilDesc, _depthHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void Frame::SetDirectQueue(ComPtr<ID3D12CommandQueue> directQueue)
{
    _queueStream = directQueue;
}

ComPtr<ID3D12CommandQueue> Frame::GetDirectQueue() const
{
    return _queueStream;
}

void Frame::SetComputeQueue(ComPtr<ID3D12CommandQueue> computeQueue)
{
    _queueCompute = computeQueue;
}

ComPtr<ID3D12CommandQueue> Frame::GetComputeQueue() const
{
    return _queueCompute;
}

void Frame::SetCopyQueue(ComPtr<ID3D12CommandQueue> copyQueue)
{
    _queueCopy = copyQueue;
}

ComPtr<ID3D12CommandQueue> Frame::GetCopyQueue() const
{
    return _queueCopy;
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

void Frame::SetSyncFrame(Fence* syncFrame)
{
    _syncFrame = syncFrame;
}

Fence* Frame::GetSyncFrame() const
{
    return _syncFrame;
}
