#pragma once

class Executor
{
public:
    void Allocate(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type);
    void Reset(ComPtr<ID3D12PipelineState> pipeline = nullptr);

    bool IsFree() const;
    void SetFree(bool isFree);

    ComPtr<ID3D12GraphicsCommandList2> GetCommandList() const;

private:
    bool _isFree = true;
    ComPtr<ID3D12CommandAllocator> _allocator;
    ComPtr<ID3D12GraphicsCommandList2> _commandList;
};
