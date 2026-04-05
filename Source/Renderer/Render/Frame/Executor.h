#pragma once

#include "RHI/CommandList.h"

namespace rhi
{
    class PipelineState;
} // namespace rhi

class Executor
{
public:
    Executor(rhi::Device* device);
    Executor(const Executor&) = delete;
    Executor(Executor&&) = default;
    ~Executor();

    Executor& operator=(const Executor&) = delete;
    Executor& operator=(Executor&&) = default;

    void Allocate(rhi::CommandListType type);
    void Reset(rhi::PipelineState* pipelineState = nullptr);

    void SetFree(bool isFree);
    bool IsFree() const;

    rhi::CommandList* GetCommandList();

private:
    std::unique_ptr<rhi::CommandList> _commandList;

    bool _isFree;

    rhi::Device* _device;
};
