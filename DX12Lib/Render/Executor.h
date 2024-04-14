#pragma once

class Executor
{
public:
    Executor();
    ~Executor();

    void Allocate(D3D12_COMMAND_LIST_TYPE type);
    void Reset(ComPtr<ID3D12PipelineState> pipeline = nullptr);

    void SetFree(bool isFree);
    bool IsFree() const;

    void SetDevice(ComPtr<ID3D12Device2> device);
    ComPtr<ID3D12Device2> GetDevice() const;

    ComPtr<ID3D12GraphicsCommandList2> GetCommandList() const;

private:
    bool _isFree = true;
    ComPtr<ID3D12CommandAllocator> _allocator;
    ComPtr<ID3D12GraphicsCommandList2> _commandList;

    ComPtr<ID3D12Device2> _DXDevice;
};
