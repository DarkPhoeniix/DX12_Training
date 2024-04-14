#include "stdafx.h"
#include "Executor.h"

Executor::Executor()
    : _allocator(nullptr)
    , _commandList(nullptr)
    , _DXDevice(nullptr)
{
}

Executor::~Executor()
{
    _allocator = nullptr;
    _commandList = nullptr;
    _DXDevice = nullptr;
}

void Executor::Allocate(D3D12_COMMAND_LIST_TYPE type)
{
    _DXDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&_allocator));
    _DXDevice->CreateCommandList(0, type, _allocator.Get(), nullptr, IID_PPV_ARGS(&_commandList));
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

void Executor::SetFree(bool isFree)
{
    _isFree = isFree;
}

bool Executor::IsFree() const
{
    return _isFree;
}

void Executor::SetDevice(ComPtr<ID3D12Device2> device)
{
    _DXDevice = device;
}

ComPtr<ID3D12Device2> Executor::GetDevice() const
{
    return _DXDevice;
}

ComPtr<ID3D12GraphicsCommandList2> Executor::GetCommandList() const
{
    return _commandList;
}
