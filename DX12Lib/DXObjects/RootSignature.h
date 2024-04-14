#pragma once

// TODO: Implement parsing of Compute graphics pipeline

class RootSignature
{
public:
    ComPtr<ID3D12RootSignature> GetRootSignature() const;
    ComPtr<ID3D12PipelineState> GetPipelineState() const;

    bool IsGraphicsPipeline() const;

    void Parse(ComPtr<ID3D12Device2> device, const std::string& filepath);

    static D3D12_BLEND_DESC ParseBlendDescription(const std::string& filepath);
    static D3D12_RASTERIZER_DESC ParseRasterizerDescription(const std::string& filepath);
    static D3D12_DEPTH_STENCIL_DESC ParseDepthStencilDescription(const std::string& filepath);

private:
    ComPtr<ID3D12RootSignature> _rootSignature;
    ComPtr<ID3D12PipelineState> _pipelineState;

    bool _isGraphicsPipeline;
};
