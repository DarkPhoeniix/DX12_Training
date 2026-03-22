
#include "RHI_PCH.h"

#include "D3D12CommandSignature.h"

namespace rhi::d3d12
{
    void CommandSignature::AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc)
    {
        _arguments.push_back(argumentDesc);
    }

    ComPtr<ID3D12CommandSignature> CommandSignature::GetDXCommandSignature() const
    {
        return _commandSignature;
    }
} // namespace rhi::d3d12
