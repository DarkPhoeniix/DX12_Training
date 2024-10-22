#include "stdafx.h"

#include "Frame.h"

#include "DXObjects/CommandList.h"
#include "DXObjects/Fence.h"
#include "DXObjects/SwapChain.h"
#include "DXObjects/RootSignature.h"

Frame::Frame()
    : Index(0)
    , Prev(nullptr)
    , Next(nullptr)
    , _swapChainTexture{}
    , _targetTexture{}
    , _depthTexture{}
    , _currentTasks{}
    , _executedTasks{}
    , _queueCompute(Core::Device::GetComputeQueue())
    , _queueStream(Core::Device::GetStreamQueue())
    , _queueCopy(Core::Device::GetCopyQueue())
    , _allocatorPool(nullptr)
    , _fencePool(nullptr)
    , _syncFrame(nullptr)
    , _tasks{}
{
}

Frame::~Frame()
{
    Prev = nullptr;
    Next = nullptr;

    _targetHeap = nullptr;
    _depthHeap = nullptr;

    _queueCompute = nullptr;
    _queueStream = nullptr;
    _queueCopy = nullptr;

    _allocatorPool = nullptr;
    _fencePool = nullptr;
    _syncFrame = nullptr;
}

void Frame::Init(const Core::SwapChain& swapChain)
{
    swapChain.GetBuffer(Index, _swapChainTexture);
    _swapChainTexture.SetName(std::string("Frame's swapchain resource ") + std::to_string(Index));

    // Create resource for the target texture
    {
        Core::ResourceDescription desc = _swapChainTexture.GetResourceDescription();

        D3D12_CLEAR_VALUE clearValueTexTarget;
        clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        clearValueTexTarget.Color[0] = 0.0f; 
        clearValueTexTarget.Color[1] = 0.0f;
        clearValueTexTarget.Color[2] = 0.0f;
        clearValueTexTarget.Color[3] = 1.0f;

        desc.AddFlags(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        desc.SetClearValue(clearValueTexTarget);

        _targetTexture.SetResourceDescription(desc);
        _targetTexture.CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_SOURCE);
        _targetTexture.SetName(std::string("Frame RTT ") + std::to_string(Index));
    }

    // Create resource for the depth texture
    {
        Core::ResourceDescription desc = _swapChainTexture.GetResourceDescription();

        desc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        desc.SetFormat(DXGI_FORMAT_D32_FLOAT);

        D3D12_CLEAR_VALUE clearValue;
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth = 1;
        clearValue.DepthStencil.Stencil = 0;

        desc.SetClearValue(clearValue);

        _depthTexture.SetResourceDescription(desc);
        _depthTexture.CreateCommitedResource(D3D12_RESOURCE_STATE_DEPTH_WRITE);
        _depthTexture.SetName(std::string("Frame DSV ") + std::to_string( Index));
    }

    // Create descriptor heaps for RTT/DSV heaps
    {
        D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
        descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        Core::Device::GetDXDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_targetHeap));

        descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        Core::Device::GetDXDevice()->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_depthHeap));
    }

    // Create RTT heap
    {
        D3D12_RENDER_TARGET_VIEW_DESC renderTargetDesc = {};
        renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        renderTargetDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        renderTargetDesc.Texture2D.MipSlice = 0;
        Core::Device::GetDXDevice()->CreateRenderTargetView(_targetTexture.GetDXResource().Get(), &renderTargetDesc, _targetHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create DSV heap
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;//D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Texture2D.MipSlice = 0;
        Core::Device::GetDXDevice()->CreateDepthStencilView(_depthTexture.GetDXResource().Get(), &depthStencilDesc, _depthHeap->GetCPUDescriptorHandleForHeapStart());
    }


    {
        Core::DescriptorHeapDescription desc = {};
        desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
        desc.SetNumDescriptors(4);
        desc.SetNodeMask(0);

        _postFXDescHeap.SetDescription(desc);
        _postFXDescHeap.Create();


        _postFXDescHeap.PlaceResource(&_targetTexture);
        _postFXDescHeap.PlaceResource(&_depthTexture);

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        UAVDesc.Texture2D.MipSlice = 0;

        Core::Device::GetDXDevice()->CreateUnorderedAccessView(_targetTexture.GetDXResource().Get(), nullptr, &UAVDesc, _postFXDescHeap.GetResourceCPUHandle(&_targetTexture));
        
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
        SRVDesc.Texture2D.MipLevels = 1;
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        Core::Device::GetDXDevice()->CreateShaderResourceView(_depthTexture.GetDXResource().Get(), &SRVDesc, _postFXDescHeap.GetResourceCPUHandle(&_depthTexture));
    }
}

TaskGPU* Frame::CreateTask(D3D12_COMMAND_LIST_TYPE type, Core::RootSignature* rootSignature)
{
    Executor* exec = _allocatorPool->Obtain(type);
    _currentTasks.push_back(exec);

    exec->Reset(rootSignature);
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

    task->AddCommandList(exec->GetCommandList());

    Core::Fence* taskFence = _fencePool->Obtain();
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

void Frame::SetSyncFrame(Core::Fence* syncFrame)
{
    _syncFrame = syncFrame;
}

Core::Fence* Frame::GetSyncFrame() const
{
    return _syncFrame;
}
