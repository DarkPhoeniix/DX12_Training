#pragma once

#include "DXObjects/CommandList.h"

namespace Core
{
    class RootSignature;
} // namespace Core

class Executor
{
public:
    Executor();
    ~Executor();

    void Allocate(D3D12_COMMAND_LIST_TYPE type);
    void Reset(Core::RootSignature* rootSignature = nullptr);

    void SetFree(bool isFree);
    bool IsFree() const;

    Core::CommandList* GetCommandList();

private:
    ComPtr<ID3D12CommandAllocator> _allocator;
    Core::CommandList _commandList;

    bool _isFree = true;
};
