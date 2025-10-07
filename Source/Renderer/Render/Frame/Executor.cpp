#include "RendererPCH.h"

#include "Executor.h"

Executor::Executor()
    : _allocator(nullptr)
    , _commandList()
    , _isFree(true)
{
}

Executor::~Executor()
{
    _allocator = nullptr;
}

void Executor::Allocate(D3D12_COMMAND_LIST_TYPE type)
{
    HRESULT createAllocatorResult = dx12::Device::GetDXDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&_allocator));
    CHECK(createAllocatorResult, "Failed to create command allocator.");

    ComPtr<ID3D12GraphicsCommandList7> commandList;
    HRESULT createCmdListResult = dx12::Device::GetDXDevice()->CreateCommandList(0, type, _allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    CHECK(createCmdListResult, "Failed to create command list.");

    _commandList.SetDXCommandList(commandList);
    _commandList.Close();
}

void Executor::Reset(dx12::PipelineState* rootSignature)
{
    //if (isFree)
    //{
    //    return;
    //}

    ID3D12PipelineState* pipelineState = rootSignature ? rootSignature->GetPipelineState().Get() : nullptr;

    HRESULT result = _allocator->Reset();
    CHECK(result, "Failed to reset command allocator.");

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

dx12::CommandList* Executor::GetCommandList()
{
    return &_commandList;
}
