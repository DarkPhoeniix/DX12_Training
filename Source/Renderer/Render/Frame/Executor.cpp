#include "RendererPCH.h"

#include "Executor.h"

Executor::Executor(rhi::Device* device)
    : _commandList(nullptr)
    , _isFree(true)
    , _device(device)
{
}

Executor::~Executor()
{
}

void Executor::Allocate(rhi::CommandListType type)
{
    _commandList = _device->CreateCommandList(type);
    _commandList->Close();
}

void Executor::Reset(rhi::PipelineState* pipelineState)
{
    // TODO: why?
    //if (isFree)
    //{
    //    return;
    //}

    _commandList->Reset(pipelineState);
}

void Executor::SetFree(bool isFree)
{
    _isFree = isFree;
}

bool Executor::IsFree() const
{
    return _isFree;
}

rhi::CommandList* Executor::GetCommandList()
{
    return _commandList.get();
}
