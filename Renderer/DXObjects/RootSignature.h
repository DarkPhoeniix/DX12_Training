#pragma once

// TODO: Implement parsing of Compute graphics pipeline

namespace Core
{
    class RootSignature
    {
    public:
        ComPtr<ID3D12RootSignature> GetRootSignature() const;
        ComPtr<ID3D12PipelineState> GetPipelineState() const;

        bool IsGraphicsPipeline() const;

        void Parse(const std::string& filepath);

        static D3D12_BLEND_DESC ParseBlendDescription(const std::string& filepath);
        static D3D12_RASTERIZER_DESC ParseRasterizerDescription(const std::string& filepath);
        static D3D12_DEPTH_STENCIL_DESC ParseDepthStencilDescription(const std::string& filepath);

    private:
        void ParseGraphicsPipeline(Json::Value& fileRoot);
        void ParseComputePipeline(Json::Value& fileRoot);

        ComPtr<ID3D12RootSignature> _rootSignature;
        ComPtr<ID3D12PipelineState> _pipelineState;

        bool _isGraphicsPipeline;
    };
} // namespace Core

