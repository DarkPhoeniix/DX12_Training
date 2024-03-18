#include "stdafx.h"

#include "PipelineSettings.h"

#include <fstream>

namespace
{
    const std::map<std::string, DXGI_FORMAT> FORMAT =
    {
        { "DXGI_FORMAT_R32G32B32_FLOAT", DXGI_FORMAT_R32G32B32_FLOAT }
    };

    DXGI_FORMAT ParseFormat(const std::string& str)
    {
        auto it = FORMAT.find(str);
        if (it != FORMAT.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the tech description");
        return FORMAT.begin()->second;
    }

    // Blend Description

    const std::map<std::string, D3D12_BLEND> DEPTH =
    {
        { "D3D12_BLEND_ZERO", D3D12_BLEND_ZERO },
        { "D3D12_BLEND_ONE", D3D12_BLEND_ONE },
        { "D3D12_BLEND_SRC_COLOR", D3D12_BLEND_SRC_COLOR },
        { "D3D12_BLEND_INV_SRC_COLOR", D3D12_BLEND_INV_SRC_COLOR },
        { "D3D12_BLEND_SRC_ALPHA", D3D12_BLEND_SRC_ALPHA },
        { "D3D12_BLEND_INV_SRC_ALPHA", D3D12_BLEND_INV_SRC_ALPHA },
        { "D3D12_BLEND_DEST_ALPHA", D3D12_BLEND_DEST_ALPHA },
        { "D3D12_BLEND_INV_DEST_ALPHA", D3D12_BLEND_INV_DEST_ALPHA },
        { "D3D12_BLEND_DEST_COLOR", D3D12_BLEND_DEST_COLOR },
        { "D3D12_BLEND_INV_DEST_COLOR", D3D12_BLEND_INV_DEST_COLOR },
        { "D3D12_BLEND_SRC_ALPHA_SAT", D3D12_BLEND_SRC_ALPHA_SAT },
        { "D3D12_BLEND_BLEND_FACTOR", D3D12_BLEND_BLEND_FACTOR },
        { "D3D12_BLEND_INV_BLEND_FACTOR", D3D12_BLEND_INV_BLEND_FACTOR },
        { "D3D12_BLEND_SRC1_COLOR", D3D12_BLEND_SRC1_COLOR },
        { "D3D12_BLEND_INV_SRC1_COLOR", D3D12_BLEND_INV_SRC1_COLOR },
        { "D3D12_BLEND_SRC1_ALPHA", D3D12_BLEND_SRC1_ALPHA },
        { "D3D12_BLEND_INV_SRC1_ALPHA", D3D12_BLEND_INV_SRC1_ALPHA },
//        { "D3D12_BLEND_ALPHA_FACTOR", D3D12_BLEND_ALPHA_FACTOR },
//        { "D3D12_BLEND_INV_ALPHA_FACTOR", D3D12_BLEND_INV_ALPHA_FACTOR }
    };

    const std::map<std::string, D3D12_BLEND_OP> BLEND_OP =
    {
        { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_ADD },
        { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_SUBTRACT },
        { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_REV_SUBTRACT },
        { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_MIN },
        { "D3D12_BLEND_OP_ADD", D3D12_BLEND_OP_MAX }
    };

    const std::map<std::string, D3D12_COLOR_WRITE_ENABLE> COLOR_WRITE =
    {
        { "D3D12_COLOR_WRITE_ENABLE_RED", D3D12_COLOR_WRITE_ENABLE_RED },
        { "D3D12_COLOR_WRITE_ENABLE_GREEN", D3D12_COLOR_WRITE_ENABLE_GREEN },
        { "D3D12_COLOR_WRITE_ENABLE_BLUE", D3D12_COLOR_WRITE_ENABLE_BLUE },
        { "D3D12_COLOR_WRITE_ENABLE_ALPHA", D3D12_COLOR_WRITE_ENABLE_ALPHA },
        { "D3D12_COLOR_WRITE_ENABLE_ALL", D3D12_COLOR_WRITE_ENABLE_ALL }
    };

    const std::map<std::string, D3D12_LOGIC_OP> LOGIC_OP =
    {
        { "D3D12_LOGIC_OP_CLEAR", D3D12_LOGIC_OP_CLEAR },
        { "D3D12_LOGIC_OP_SET", D3D12_LOGIC_OP_SET },
        { "D3D12_LOGIC_OP_COPY", D3D12_LOGIC_OP_COPY },
        { "D3D12_LOGIC_OP_COPY_INVERTED", D3D12_LOGIC_OP_COPY_INVERTED },
        { "D3D12_LOGIC_OP_NOOP", D3D12_LOGIC_OP_NOOP },
        { "D3D12_LOGIC_OP_INVERT", D3D12_LOGIC_OP_INVERT },
        { "D3D12_LOGIC_OP_AND", D3D12_LOGIC_OP_AND },
        { "D3D12_LOGIC_OP_NAND", D3D12_LOGIC_OP_NAND },
        { "D3D12_LOGIC_OP_OR", D3D12_LOGIC_OP_OR },
        { "D3D12_LOGIC_OP_NOR", D3D12_LOGIC_OP_NOR },
        { "D3D12_LOGIC_OP_XOR", D3D12_LOGIC_OP_XOR },
        { "D3D12_LOGIC_OP_EQUIV", D3D12_LOGIC_OP_EQUIV },
        { "D3D12_LOGIC_OP_AND_REVERSE", D3D12_LOGIC_OP_AND_REVERSE },
        { "D3D12_LOGIC_OP_AND_INVERTED", D3D12_LOGIC_OP_AND_INVERTED },
        { "D3D12_LOGIC_OP_OR_REVERSE", D3D12_LOGIC_OP_OR_REVERSE },
        { "D3D12_LOGIC_OP_OR_INVERTED", D3D12_LOGIC_OP_OR_INVERTED }
    };

    D3D12_BLEND ParseBlend(const std::string& str)
    {
        auto it = DEPTH.find(str);
        if (it != DEPTH.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the blend description");
        return DEPTH.begin()->second;
    }

    D3D12_BLEND_OP ParseBlendOp(const std::string& str)
    {
        auto it = BLEND_OP.find(str);
        if (it != BLEND_OP.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the blend description");
        return BLEND_OP.begin()->second;
    }

    D3D12_COLOR_WRITE_ENABLE ParseColorWriteEnable(const std::string& str)
    {
        auto it = COLOR_WRITE.find(str);
        if (it != COLOR_WRITE.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the blend description");
        return COLOR_WRITE.begin()->second;
    }

    D3D12_LOGIC_OP ParseLogicOp(const std::string& str)
    {
        auto it = LOGIC_OP.find(str);
        if (it != LOGIC_OP.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the blend description");
        return LOGIC_OP.begin()->second;
    }

    // Rasterizer Description

    const std::map<std::string, D3D12_FILL_MODE> FILL_MODE =
    {
        { "D3D12_FILL_MODE_WIREFRAME", D3D12_FILL_MODE_WIREFRAME },
        { "D3D12_FILL_MODE_SOLID", D3D12_FILL_MODE_SOLID }
    };

    const std::map<std::string, D3D12_CULL_MODE> CULL_MODE =
    {
        { "D3D12_CULL_MODE_NONE", D3D12_CULL_MODE_NONE },
        { "D3D12_CULL_MODE_FRONT", D3D12_CULL_MODE_FRONT },
        { "D3D12_CULL_MODE_BACK", D3D12_CULL_MODE_BACK }
    };

    D3D12_FILL_MODE ParseFillMode(const std::string& str)
    {
        auto it = FILL_MODE.find(str);
        if (it != FILL_MODE.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the raster description");
        return FILL_MODE.begin()->second;
    }

    D3D12_CULL_MODE ParseCullMode(const std::string& str)
    {
        auto it = CULL_MODE.find(str);
        if (it != CULL_MODE.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the raster description");
        return CULL_MODE.begin()->second;
    }

    // Depth Stencil Description

    const std::map<std::string, D3D12_COMPARISON_FUNC> COMPARISON_FUNC =
    {
        { "D3D12_COMPARISON_FUNC_NEVER", D3D12_COMPARISON_FUNC_NEVER },
        { "D3D12_COMPARISON_FUNC_LESS", D3D12_COMPARISON_FUNC_LESS },
        { "D3D12_COMPARISON_FUNC_EQUAL", D3D12_COMPARISON_FUNC_EQUAL },
        { "D3D12_COMPARISON_FUNC_LESS_EQUAL", D3D12_COMPARISON_FUNC_LESS_EQUAL },
        { "D3D12_COMPARISON_FUNC_GREATER", D3D12_COMPARISON_FUNC_GREATER },
        { "D3D12_COMPARISON_FUNC_NOT_EQUAL", D3D12_COMPARISON_FUNC_NOT_EQUAL },
        { "D3D12_COMPARISON_FUNC_GREATER_EQUAL", D3D12_COMPARISON_FUNC_GREATER_EQUAL },
        { "D3D12_COMPARISON_FUNC_ALWAYS", D3D12_COMPARISON_FUNC_ALWAYS }
    };

    const std::map<std::string, D3D12_DEPTH_WRITE_MASK> DEPTH_WRITE_MASK =
    {
        { "D3D12_DEPTH_WRITE_MASK_ZERO", D3D12_DEPTH_WRITE_MASK_ZERO },
        { "D3D12_DEPTH_WRITE_MASK_ALL", D3D12_DEPTH_WRITE_MASK_ALL }
    };

    D3D12_COMPARISON_FUNC ParseComparisonFunc(const std::string& str)
    {
        auto it = COMPARISON_FUNC.find(str);
        if (it != COMPARISON_FUNC.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the depth stencil description");
        return COMPARISON_FUNC.begin()->second;
    }

    D3D12_DEPTH_WRITE_MASK ParseDepthWriteMask(const std::string& str)
    {
        auto it = DEPTH_WRITE_MASK.find(str);
        if (it != DEPTH_WRITE_MASK.end())
        {
            return it->second;
        }
        Logger::Log(LogType::Warning, "Failed to parse " + str + " from the depth stencil description");
        return DEPTH_WRITE_MASK.begin()->second;
    }
}

void PipelineSettings::Parse(ID3D12Device* device, const std::string& filepath)
{
    //const std::string pipelineFilepath = "Resources\\RenderPipeline.tech";
    Json::Value jsonRoot = Helper::ParseJson(filepath);

    // Load the vertex shader
    std::string vertexShaderFilepath = jsonRoot["VS"].asString();
    ComPtr<ID3DBlob> vertexShaderBlob;
    Helper::throwIfFailed(D3DReadFileToBlob(std::wstring(vertexShaderFilepath.begin(), vertexShaderFilepath.end()).c_str(), &vertexShaderBlob));

    // Load the pixel shader
    std::string pixelShaderFilepath = jsonRoot["PS"].asString();
    ComPtr<ID3DBlob> pixelShaderBlob;
    Helper::throwIfFailed(D3DReadFileToBlob(std::wstring(pixelShaderFilepath.begin(), pixelShaderFilepath.end()).c_str(), &pixelShaderBlob));

    // Create the vertex input layout
    // TODO implement parsing the layout from the "RenderPipeline.tech" 
    D3D12_INPUT_ELEMENT_DESC inputLayout[4] = {};
    std::vector<std::string> names(4);
    for (int i = 0; i < 4; ++i)
    {
        //inputLayout[i] = {};
        Json::Value layout = jsonRoot["layout"][i];
        names[i] = layout["name"].asString();
        inputLayout[i].SemanticName         = names[i].c_str();
        inputLayout[i].SemanticIndex        = layout["semanticIndex"].asUInt();
        inputLayout[i].Format               = ParseFormat(layout["format"].asString());
        inputLayout[i].AlignedByteOffset    = layout["offset"].asUInt();
        inputLayout[i].InputSlot            = layout["stream"].asUInt();
    }

    Helper::throwIfFailed(device->CreateRootSignature(0, vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

    const std::string blendPipelineDescFilepath = jsonRoot["blend"].asString();
    const std::string rasterPipelineDescFilepath = jsonRoot["raster"].asString();
    const std::string depthPipelineDescFilepath = jsonRoot["depth"].asString();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateStreamDesc = {};

    pipelineStateStreamDesc.BlendState = ParseBlendDescription(blendPipelineDescFilepath);
    pipelineStateStreamDesc.RasterizerState = ParseRasterizerDescription(rasterPipelineDescFilepath);
    pipelineStateStreamDesc.DepthStencilState = ParseDepthStencilDescription(depthPipelineDescFilepath);

    pipelineStateStreamDesc.pRootSignature = _rootSignature.Get();
    pipelineStateStreamDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStreamDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStreamDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStreamDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStreamDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStreamDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateStreamDesc.NumRenderTargets = 1;
    pipelineStateStreamDesc.SampleDesc.Count = 1; // must be the same sample description as the swapChain and depth/stencil buffer
    pipelineStateStreamDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done

    Helper::throwIfFailed(device->CreateGraphicsPipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&_pipelineState)));
}

ComPtr<ID3D12RootSignature> PipelineSettings::GetRootSignature() const
{
    return _rootSignature;
}

ComPtr<ID3D12PipelineState> PipelineSettings::GetPipelineState() const
{
    return _pipelineState;
}

bool PipelineSettings::IsGraphicsPipeline() const
{
    return _isGraphicsPipeline;
}

D3D12_BLEND_DESC PipelineSettings::ParseBlendDescription(const std::string& filepath)
{
    Json::Value root = Helper::ParseJson(filepath);
    Json::Value renderTargets = root["RenderTargets"];

    D3D12_BLEND_DESC description = {};

    for (size_t i = 0; i < renderTargets.size(); ++i)
    {
        description.RenderTarget[i].BlendEnable = root["BlendEnable"].asBool();
        description.RenderTarget[i].SrcBlend = ParseBlend(root["SrcBlend"].asString());
        description.RenderTarget[i].DestBlend = ParseBlend(root["DestBlend"].asString());
        description.RenderTarget[i].BlendOp = ParseBlendOp(root["BlendOp"].asString());

        description.RenderTarget[i].SrcBlendAlpha = ParseBlend(root["SrcBlendAlpha"].asString());
        description.RenderTarget[i].DestBlendAlpha = ParseBlend(root["DestBlendAlpha"].asString());
        description.RenderTarget[i].BlendOpAlpha = ParseBlendOp(root["BlendOpAlpha"].asString());

        description.RenderTarget[i].RenderTargetWriteMask = ParseColorWriteEnable(root["RenderTargetWriteMask"].asString());

        description.RenderTarget[i].LogicOpEnable = root["LogicOpEnable"].asBool();
        description.RenderTarget[i].LogicOp = ParseLogicOp(root["LogicOp"].asString());
    }

    return description;
}

D3D12_RASTERIZER_DESC PipelineSettings::ParseRasterizerDescription(const std::string& filepath)
{
    Json::Value root = Helper::ParseJson(filepath);

    D3D12_RASTERIZER_DESC description = {};
    description.FillMode = ParseFillMode(root["FillMode"].asString());
    description.CullMode = ParseCullMode(root["CullMode"].asString());
    description.DepthClipEnable = root["DepthClipEnable"].asBool();

    return description;
}

D3D12_DEPTH_STENCIL_DESC PipelineSettings::ParseDepthStencilDescription(const std::string& filepath)
{
    Json::Value root = Helper::ParseJson(filepath);

    D3D12_DEPTH_STENCIL_DESC description = {};
    description.DepthEnable = root["DepthEnable"].asBool();
    description.DepthFunc = ParseComparisonFunc(root["DepthFunc"].asString());
    description.DepthWriteMask = ParseDepthWriteMask(root["DepthWriteMask"].asString());
    description.StencilEnable = root["StencilEnable"].asBool();

    return description;
}
