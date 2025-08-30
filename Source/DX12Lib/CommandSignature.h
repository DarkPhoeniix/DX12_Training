#pragma once

#include "PipelineState.h"

namespace dx12
{
    class CommandSignature
    {
    public:
        CommandSignature() = default;
        ~CommandSignature() = default;

        void Create();

        void AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc);

    private:
        PipelineState& _pipelineState;

        std::vector<D3D12_INDIRECT_ARGUMENT_DESC> _arguments;
        ComPtr<ID3D12CommandSignature> _commandSignature;
    };
} // namespace dx12
