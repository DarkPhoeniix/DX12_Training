#include "DX12LibPCH.h"

#include "CommandSignature.h"

namespace dx12
{
    void CommandSignature::Create(std::uint32_t size, PipelineState* pipelineState)
    {
        D3D12_COMMAND_SIGNATURE_DESC desc = {};
        desc.NumArgumentDescs = _arguments.size();
        desc.pArgumentDescs = _arguments.data();
        desc.ByteStride = size;

        ID3D12RootSignature* rootSignature = pipelineState ? pipelineState->GetRootSignature().Get() : nullptr;
        dx12::Device::GetDXDevice()->CreateCommandSignature(&desc, rootSignature, IID_PPV_ARGS(&_commandSignature));
    }

    void CommandSignature::AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc)
    {
        _arguments.push_back(argumentDesc);
    }

    ComPtr<ID3D12CommandSignature> CommandSignature::GetDXCommandSignature() const
    {
        return _commandSignature;
    }
} // namespace dx12
