#pragma once

#include "PipelineState.h"

namespace dx12
{
    class CommandSignature
    {
    public:
        CommandSignature() = default;
        CommandSignature(const CommandSignature& other) = delete;
        CommandSignature(CommandSignature&& other) noexcept = default;
        ~CommandSignature() = default;

        CommandSignature& operator=(const CommandSignature& other) = delete;
        CommandSignature& operator=(CommandSignature&& other) noexcept = default;

        void Create(std::uint32_t size, PipelineState* pipelineState = nullptr);

        void AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc);

        ComPtr<ID3D12CommandSignature> GetDXCommandSignature() const;

    private:
        std::vector<D3D12_INDIRECT_ARGUMENT_DESC> _arguments;
        ComPtr<ID3D12CommandSignature> _commandSignature;
    };
} // namespace dx12
