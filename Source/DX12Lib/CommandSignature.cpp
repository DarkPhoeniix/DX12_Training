#include "DX12LibPCH.h"

#include "CommandSignature.h"

namespace dx12
{
    void CommandSignature::Create()
    {
        D3D12_COMMAND_SIGNATURE_DESC desc = {};
        desc.NumArgumentDescs = _arguments.size();
        desc.pArgumentDescs = _arguments.data();

        dx12::Device::GetDXDevice()->CreateCommandSignature(&desc, _pipelineState.GetRootSignature().Get(), IID_PPV_ARGS(&_commandSignature));
    }

    void CommandSignature::AddArgument(D3D12_INDIRECT_ARGUMENT_DESC argumentDesc)
    {
        _arguments.push_back(argumentDesc);
    }
}
