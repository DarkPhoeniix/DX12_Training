#pragma once

#include "PipelineState.h"

namespace Json
{
    class Value;
} // namespace Json

namespace rhi::d3d12
{
    class D3D12PipelineState final : public PipelineState
    {
    public:
        D3D12PipelineState(Device* device, const std::string& filepath);
        D3D12PipelineState(const D3D12PipelineState& other) = delete;
        D3D12PipelineState(D3D12PipelineState&& other) noexcept;
        ~D3D12PipelineState() override;

        D3D12PipelineState& operator=(const D3D12PipelineState& other) = delete;
        D3D12PipelineState& operator=(D3D12PipelineState&& other) noexcept;

        ComPtr<ID3D12RootSignature> GetRootSignature() const;
        ComPtr<ID3D12PipelineState> GetPipelineState() const;

        PipelineStateType GetType() const override;

        void* GetNative() const override;
        void* GetNativeRootSignature() const override;

    private:
        friend class D3D12Device;

        D3D12PipelineState(Device* device);

        void Parse(const std::string& filepath);
        void ParseGraphicsPipeline(const Json::Value& fileRoot);
        void ParseComputePipeline(const Json::Value& fileRoot);

        BlendState ParseBlendDescription(const std::string& filepath);
        RasterizerState ParseRasterizerDescription(const std::string& filepath);
        DepthStencilState ParseDepthStencilDescription(const std::string& filepath);

        ComPtr<ID3D12RootSignature> _rootSignature;
        ComPtr<ID3D12PipelineState> _pipelineState;

        PipelineStateType _type;

        Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
