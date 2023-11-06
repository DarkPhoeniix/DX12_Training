#include "stdafx.h"
#include "PipelineSettingsParser.h"

#include <json/reader.h>

#include <fstream>

// TODO: add logs to the file

namespace
{
    Json::Value ParseJson(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios_base::binary);

        // TODO: handle error / add logs

        Json::Value root;
        file >> root;

        return root;
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
        { "D3D12_BLEND_ALPHA_FACTOR", D3D12_BLEND_ALPHA_FACTOR },
        { "D3D12_BLEND_INV_ALPHA_FACTOR", D3D12_BLEND_INV_ALPHA_FACTOR }
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
        return DEPTH.begin()->second;
    }

    D3D12_BLEND_OP ParseBlendOp(const std::string& str)
    {
        auto it = BLEND_OP.find(str);
        if (it != BLEND_OP.end())
        {
            return it->second;
        }
        return BLEND_OP.begin()->second;
    }

    D3D12_COLOR_WRITE_ENABLE ParseColorWriteEnable(const std::string& str)
    {
        auto it = COLOR_WRITE.find(str);
        if (it != COLOR_WRITE.end())
        {
            return it->second;
        }
        return COLOR_WRITE.begin()->second;
    }

    D3D12_LOGIC_OP ParseLogicOp(const std::string& str)
    {
        auto it = LOGIC_OP.find(str);
        if (it != LOGIC_OP.end())
        {
            return it->second;
        }
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
        return FILL_MODE.begin()->second;
    }

    D3D12_CULL_MODE ParseCullMode(const std::string& str)
    {
        auto it = CULL_MODE.find(str);
        if (it != CULL_MODE.end())
        {
            return it->second;
        }
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
        return COMPARISON_FUNC.begin()->second;
    }

    D3D12_DEPTH_WRITE_MASK ParseDepthWriteMask(const std::string& str)
    {
        auto it = DEPTH_WRITE_MASK.find(str);
        if (it != DEPTH_WRITE_MASK.end())
        {
            return it->second;
        }
        return DEPTH_WRITE_MASK.begin()->second;
    }
}

D3D12_BLEND_DESC PipelineSettingsParser::ParseBlendDescription(const std::string& filepath)
{
    Json::Value root = ParseJson(filepath);
    Json::Value renderTargets = root["RenderTargets"];

    D3D12_BLEND_DESC description;

    for (size_t i = 0; i < renderTargets.size(); ++i)
    {
        description.RenderTarget[i].BlendEnable = root["BlendEnbale"].asBool();
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

D3D12_RASTERIZER_DESC PipelineSettingsParser::ParseRasterizerDescription(const std::string& filepath)
{
    Json::Value root = ParseJson(filepath);

    D3D12_RASTERIZER_DESC description = {};
    description.FillMode = ParseFillMode(root["FillMode"].asString());
    description.CullMode = ParseCullMode(root["CullMode"].asString());
    description.DepthClipEnable = root["DepthClipEnable"].asBool();

    return description;
}

D3D12_DEPTH_STENCIL_DESC PipelineSettingsParser::ParseDepthStencilDescription(const std::string& filepath)
{
    Json::Value root = ParseJson(filepath);

    D3D12_DEPTH_STENCIL_DESC description = {};
    description.DepthEnable = root["DepthEnable"].asBool();
    description.DepthFunc = ParseComparisonFunc(root["DepthFunc"].asString());
    description.DepthWriteMask = ParseDepthWriteMask(root["DepthWriteMask"].asString());
    description.StencilEnable = root["StencilEnable"].asBool();

    return description;
}
