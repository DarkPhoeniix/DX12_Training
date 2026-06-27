
#include "RHI_PCH.h"

#include "D3D12CommandSignature.h"

#include "D3D12Helpers.h"

#include "PipelineState.h"

namespace rhi::d3d12
{
    namespace
    {
        std::uint32_t GetArgumentSize(const IndirectArgumentDescription& argument)
        {
            switch (argument.Type)
            {
            case IndirectArgumentType::Draw:
                return 16; // VertexCountPerInstance(4) + InstanceCount(4) + StartVertexLocation(4) + StartInstanceLocation(4)
            case IndirectArgumentType::DrawIndexed:
                return 20; // IndexCountPerInstance(4) + InstanceCount(4) + StartIndexLocation(4) + BaseVertexLocation(4) + StartInstanceLocation(4)
            case IndirectArgumentType::Dispatch:
                return 12; // XThreadGroups(4) + YThreadGroups(4) + ZThreadGroups(4)
            case IndirectArgumentType::VertexBufferView:
                return 16; // VirtualSddress(8) + Stride(4) + Size(4)
            case IndirectArgumentType::IndexBufferView:
                return 16; // VirtualSddress(8) + Size(4) + Format(4)
            case IndirectArgumentType::Constant:
                return 4 * argument.Constant.Num32BitValuesToSet; // Data(4) * numValues
            case IndirectArgumentType::ConstantBufferView:
                return 8; // VirtualSddress(8)
            case IndirectArgumentType::ShaderResourceView:
                return 8; // VirtualSddress(8)
            case IndirectArgumentType::UnorderedResourceView:
                return 8; // VirtualSddress(8)
            default:
                UNREACHABLE("Unsupported indirect argument type.");
                return 0;
            }
        }

        std::uint32_t CalculateCommandSignatureSize(const std::vector<IndirectArgumentDescription>& arguments)
        {
            std::uint32_t size = 0;

            for (const auto& arg : arguments)
            {
                size += GetArgumentSize(arg);
            }
            size = Math::AlignUp(size, 16);

            return size;
        }
    } // namespace unnamed

    D3D12CommandSignature::D3D12CommandSignature(Device* device, const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name)
#if ENABLE_DEBUG_DESC
        : _arguments(arguments)
#endif // ENABLE_DEBUG_DESC
    {
        std::uint32_t argsCount = static_cast<std::uint32_t>(arguments.size());

        D3D12_INDIRECT_ARGUMENT_DESC* args = new D3D12_INDIRECT_ARGUMENT_DESC[argsCount];
        for (size_t i = 0; i < argsCount; ++i)
        {
            args[i] = GetD3D12IndirectArgumentDesc(arguments[i]);
        }

        D3D12_COMMAND_SIGNATURE_DESC desc =
        {
            .ByteStride = CalculateCommandSignatureSize(arguments),
            .NumArgumentDescs = argsCount,
            .pArgumentDescs = args,
            .NodeMask = 0
        };

        ID3D12Device2* nativeDevice = D3D12Cast<ID3D12Device2>(device->GetNative());
        ID3D12RootSignature* nativeRootSignature = pipelineState ? D3D12Cast<ID3D12RootSignature>(pipelineState->GetNativeRootSignature()) : nullptr;

        nativeDevice->CreateCommandSignature(&desc, nativeRootSignature, IID_PPV_ARGS(&_commandSignature));

        delete[] args;

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_commandSignature.Get(), name);
#endif // ENABLE_DEBUG_NAMES
    }

    D3D12CommandSignature::D3D12CommandSignature(D3D12CommandSignature&& other) noexcept
        : _commandSignature(std::move(other._commandSignature))
#if ENABLE_DEBUG_DESC
        , _arguments(std::move(other._arguments))
#endif // ENABLE_DEBUG_DESC
    {
    }

    D3D12CommandSignature& D3D12CommandSignature::operator=(D3D12CommandSignature&& other) noexcept
    {
        if (this != &other)
        {
            _commandSignature = std::move(other._commandSignature);
#if ENABLE_DEBUG_DESC
            _arguments = std::move(other._arguments);
#endif // ENABLE_DEBUG_DESC
        }

        return *this;
    }

    void* D3D12CommandSignature::GetNative() const
    {
        return static_cast<void*>(_commandSignature.Get());
    }
} // namespace rhi::d3d12
