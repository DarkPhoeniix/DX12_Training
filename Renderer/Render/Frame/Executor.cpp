#include "stdafx.h"

#include "Executor.h"

Executor::Executor()
    : _allocator(nullptr)
    , _commandList()
{
}

Executor::~Executor()
{
    _allocator = nullptr;
}

void Executor::Allocate(D3D12_COMMAND_LIST_TYPE type)
{
    Core::Device::GetDXDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&_allocator));

    ComPtr<ID3D12GraphicsCommandList> commandList;
    Core::Device::GetDXDevice()->CreateCommandList(0, type, _allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

    _commandList.SetDXCommandList(commandList);
    _commandList.Close();
}

void Executor::Reset(Core::RootSignature* rootSignature)
{
    //if (isFree)
    //{
    //    return;
    //}

    ID3D12PipelineState* pipelineState = rootSignature ? rootSignature->GetPipelineState().Get() : nullptr;

    _allocator->Reset();
    _commandList.Reset(_allocator.Get(), pipelineState);
}

void Executor::SetFree(bool isFree)
{
    _isFree = isFree;
}

bool Executor::IsFree() const
{
    return _isFree;
}

Core::CommandList* Executor::GetCommandList()
{
    return &_commandList;
}
