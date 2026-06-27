#pragma once

#include "CommandSignature.h"

namespace rhi
{
    class PipelineState;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12CommandSignature : public CommandSignature
    {
    public:
        D3D12CommandSignature(const D3D12CommandSignature& other) = delete;
        D3D12CommandSignature(D3D12CommandSignature&& other) noexcept;
        ~D3D12CommandSignature() = default;

        D3D12CommandSignature& operator=(const D3D12CommandSignature& other) = delete;
        D3D12CommandSignature& operator=(D3D12CommandSignature&& other) noexcept;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12CommandSignature(Device* device, const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name = "");

        ComPtr<ID3D12CommandSignature> _commandSignature;

#if ENABLE_DEBUG_DESC
        std::vector<IndirectArgumentDescription> _arguments;
#endif // ENABLE_DEBUG_DESC
    };
} // namespace rhi::d3d12
