#pragma once

#include <queue>

class CommandQueue
{
public:
    CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandQueue();

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> getD3D12CommandQueue() const;

    // Get an available command list from the command queue
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> getCommandList();
    // Returns the fence value to wait for this command list.
    uint64_t executeCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

    uint64_t signal();

    bool isFenceComplete(uint64_t fenceValue);
    void waitForFenceValue(uint64_t fenceValue);

    void flush();

protected:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> createCommandAllocator();
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> createCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

private:
    // Keep track of command allocators that are "in-flight"
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue< Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> >;

    D3D12_COMMAND_LIST_TYPE _commandListType;
    Microsoft::WRL::ComPtr<ID3D12Device2> _d3d12Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _d3d12CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> _d3d12Fence;
    HANDLE _fenceEvent;
    uint64_t _fenceValue;

    CommandAllocatorQueue _commandAllocatorQueue;
    CommandListQueue _commandListQueue;
};
