#include "stdafx.h"
#include "Executor.h"

void Executor::Allocate(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
{
    device->CreateCommandAllocator(type, IID_PPV_ARGS(&_allocator));
    device->CreateCommandList(0, type, _allocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));
    _commandList->Close();
}

void Executor::Reset(ComPtr<ID3D12PipelineState> pipeline)
{
    //if (isFree)
    //{
    //    return;
    //}

    ID3D12PipelineState* pState = pipeline ? pipeline.Get() : nullptr;

    _allocator->Reset();
    _commandList->Reset(_allocator.Get(), pState);
}

bool Executor::IsFree() const
{
    return _isFree;
}

void Executor::SetFree(bool isFree)
{
    _isFree = isFree;
}

ComPtr<ID3D12GraphicsCommandList2> Executor::GetCommandList() const
{
    return _commandList;
}
