#pragma once

#include "CommandList.h"

namespace dx12
{
    class PipelineState;
} // namespace Core

class Executor
{
public:
    Executor();
    ~Executor();

    void Allocate(D3D12_COMMAND_LIST_TYPE type);
    void Reset(dx12::PipelineState* rootSignature = nullptr);

    void SetFree(bool isFree);
    bool IsFree() const;

    dx12::CommandList* GetCommandList();

private:
    ComPtr<ID3D12CommandAllocator> _allocator;
    dx12::CommandList _commandList;

    bool _isFree;
};
