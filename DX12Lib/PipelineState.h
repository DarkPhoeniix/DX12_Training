#pragma once

// TODO: Implement parsing of Compute graphics pipeline

namespace dx12
{
    class PipelineState
    {
    public:
        ComPtr<ID3D12RootSignature> GetRootSignature() const;
        ComPtr<ID3D12PipelineState> GetPipelineState() const;

        bool IsGraphicsPipeline() const;

        void Parse(const std::string& filepath);

        D3D12_BLEND_DESC ParseBlendDescription(const std::string& filepath);
        D3D12_RASTERIZER_DESC ParseRasterizerDescription(const std::string& filepath);
        D3D12_DEPTH_STENCIL_DESC ParseDepthStencilDescription(const std::string& filepath);

    private:
        void ParseGraphicsPipeline(const Json::Value& fileRoot);
        void ParseComputePipeline(const Json::Value& fileRoot);

        ComPtr<ID3D12RootSignature> _rootSignature;
        ComPtr<ID3D12PipelineState> _pipelineState;

        bool _isGraphicsPipeline;
    };
} // namespace dx12

