#pragma once

class PipelineSettingsParser
{
public:
    static D3D12_BLEND_DESC ParseBlendDescription(const std::string& filepath);
    static D3D12_RASTERIZER_DESC ParseRasterizerDescription(const std::string& filepath);
    static D3D12_DEPTH_STENCIL_DESC ParseDepthStencilDescription(const std::string& filepath);
};
