#pragma once

#include "PipelineState.h"

namespace rhi::d3d12
{
    class CommandSignature
    {
    public:
        CommandSignature(const CommandSignature& other) = delete;
        CommandSignature(CommandSignature&& other) noexcept;
        ~CommandSignature() = default;

        CommandSignature& operator=(const CommandSignature& other) = delete;
        CommandSignature& operator=(CommandSignature&& other) noexcept;

        void Create(std::uint32_t size, PipelineState* pipelineState = nullptr);

        void AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc);

        ComPtr<ID3D12CommandSignature> GetDXCommandSignature() const;

    private:
        friend class D3D12Device;

        CommandSignature() = default;

        std::vector<D3D12_INDIRECT_ARGUMENT_DESC> _arguments;
        ComPtr<ID3D12CommandSignature> _commandSignature;
    };
} // namespace rhi::d3d12
