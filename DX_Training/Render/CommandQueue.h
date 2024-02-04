#pragma once

#include <queue>

class CommandQueue
{
public:
    CommandQueue(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandQueue();

    ComPtr<ID3D12CommandQueue> getD3D12CommandQueue() const;

    ComPtr<ID3D12GraphicsCommandList2> getCommandList();
    uint64_t executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList);

    uint64_t signal();

    bool isFenceComplete(uint64_t fenceValue);
    void waitForFenceValue(uint64_t fenceValue);

    void flush();

protected:
    ComPtr<ID3D12CommandAllocator> createCommandAllocator();
    ComPtr<ID3D12GraphicsCommandList2> createCommandList(ComPtr<ID3D12CommandAllocator> allocator);

private:
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue< ComPtr<ID3D12GraphicsCommandList2> >;

    ComPtr<ID3D12Device2> _d3d12Device;
    D3D12_COMMAND_LIST_TYPE _commandListType;
    ComPtr<ID3D12CommandQueue> _d3d12CommandQueue;

    ComPtr<ID3D12Fence> _d3d12Fence;
    HANDLE _fenceEvent;
    uint64_t _fenceValue;

    CommandAllocatorQueue _commandAllocatorQueue;
    CommandListQueue _commandListQueue;
};
