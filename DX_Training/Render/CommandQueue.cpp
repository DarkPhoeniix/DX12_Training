#include "stdafx.h"

#include "CommandQueue.h"

CommandQueue::CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    : _fenceValue(0)
    , _commandListType(type)
    , _d3d12Device(device)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    Helper::throwIfFailed(_d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_d3d12CommandQueue)));
    Helper::throwIfFailed(_d3d12Device->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_d3d12Fence)));

    _fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(_fenceEvent && "Failed to create fence event handle.");
    if (!_fenceEvent)
    {
        Logger::Log(LogType::Error, "Failed to create fence event handle");
    }
}

CommandQueue::~CommandQueue()
{   }

uint64_t CommandQueue::signal()
{
    uint64_t fenceValue = ++_fenceValue;
    _d3d12CommandQueue->Signal(_d3d12Fence.Get(), fenceValue);

    return fenceValue;
}

bool CommandQueue::isFenceComplete(uint64_t fenceValue)
{
    return _d3d12Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::waitForFenceValue(uint64_t fenceValue)
{
    if (!isFenceComplete(fenceValue))
    {
        _d3d12Fence->SetEventOnCompletion(fenceValue, _fenceEvent);
        ::WaitForSingleObject(_fenceEvent, DWORD_MAX);
    }
}

void CommandQueue::flush()
{
    waitForFenceValue(signal());
}

ComPtr<ID3D12CommandAllocator> CommandQueue::createCommandAllocator()
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    Helper::throwIfFailed(_d3d12Device->CreateCommandAllocator(_commandListType, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::createCommandList(ComPtr<ID3D12CommandAllocator> allocator)
{
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    Helper::throwIfFailed(_d3d12Device->CreateCommandList(0, _commandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    return commandList;
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::getCommandList()
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList2> commandList;

    if (!_commandAllocatorQueue.empty() && isFenceComplete(_commandAllocatorQueue.front().fenceValue))
    {
        commandAllocator = _commandAllocatorQueue.front().commandAllocator;
        _commandAllocatorQueue.pop();

        Helper::throwIfFailed(commandAllocator->Reset());
    }
    else
    {
        commandAllocator = createCommandAllocator();
    }

    if (!_commandListQueue.empty())
    {
        commandList = _commandListQueue.front();
        _commandListQueue.pop();

        Helper::throwIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
    }
    else
    {
        commandList = createCommandList(commandAllocator);
    }

    // Associate the command allocator with the command list so that it can be
    // retrieved when the command list is executed.
    Helper::throwIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t CommandQueue::executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandAllocator* commandAllocator;
    UINT dataSize = sizeof(commandAllocator);
    Helper::throwIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* const ppCommandLists[] = {
        commandList.Get()
    };

    _d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
    uint64_t fenceValue = signal();

    _commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
    _commandListQueue.push(commandList);

    // The ownership of the command allocator has been transferred to the ComPtr
    // in the command allocator queue. It is safe to release the reference 
    // in this temporary COM pointer here.
    commandAllocator->Release();

    return fenceValue;
}

ComPtr<ID3D12CommandQueue> CommandQueue::getD3D12CommandQueue() const
{
    return _d3d12CommandQueue;
}
