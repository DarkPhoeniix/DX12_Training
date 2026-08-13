
#include "RHI_PCH.h"

#include "PipelineHelpers.h"

#include <json/json.h>

#include <filesystem>
#include <fstream>

namespace
{
    const std::map<std::string, rhi::PipelineStateType> PIPELINE_TYPE =
    {
        { "Graphics", rhi::PipelineStateType::Graphics },
        { "Compute",  rhi::PipelineStateType::Compute  }
    };

    const std::map<std::string, rhi::Format> FORMAT =
    {
        { "float4", rhi::Format::R32G32B32A32_FLOAT },
        { "float3", rhi::Format::R32G32B32_FLOAT    },
        { "float2", rhi::Format::R32G32_FLOAT       },
        { "float",  rhi::Format::R32_FLOAT          },
        { "uint4",  rhi::Format::R32G32B32A32_UINT  },
        { "uint3",  rhi::Format::R32G32B32_UINT     },
        { "uint2",  rhi::Format::R32G32_UINT        },
        { "uint",   rhi::Format::R32_UINT           },
    };

    const std::map<std::string, rhi::Blend> BLEND =
    {
        { "D3D12_BLEND_ZERO",             rhi::Blend::Zero           },
        { "D3D12_BLEND_ONE",              rhi::Blend::One            },
        { "D3D12_BLEND_SRC_COLOR",        rhi::Blend::SrcColor       },
        { "D3D12_BLEND_INV_SRC_COLOR",    rhi::Blend::InvSrcColor    },
        { "D3D12_BLEND_SRC_ALPHA",        rhi::Blend::SrcAlpha       },
        { "D3D12_BLEND_INV_SRC_ALPHA",    rhi::Blend::InvSrcAlpha    },
        { "D3D12_BLEND_DEST_ALPHA",       rhi::Blend::DestAlpha      },
        { "D3D12_BLEND_INV_DEST_ALPHA",   rhi::Blend::InvDestAlpha   },
        { "D3D12_BLEND_DEST_COLOR",       rhi::Blend::DestColor      },
        { "D3D12_BLEND_INV_DEST_COLOR",   rhi::Blend::InvDestColor   },
        { "D3D12_BLEND_SRC_ALPHA_SAT",    rhi::Blend::SrcAlphaSat    },
        { "D3D12_BLEND_BLEND_FACTOR",     rhi::Blend::BlendFactor    },
        { "D3D12_BLEND_INV_BLEND_FACTOR", rhi::Blend::InvBlendFactor },
        { "D3D12_BLEND_SRC1_COLOR",       rhi::Blend::Src1Color      },
        { "D3D12_BLEND_INV_SRC1_COLOR",   rhi::Blend::InvSrc1Color   },
        { "D3D12_BLEND_SRC1_ALPHA",       rhi::Blend::Src1Alpha      },
        { "D3D12_BLEND_INV_SRC1_ALPHA",   rhi::Blend::InvSrc1Alpha   },
        { "D3D12_BLEND_ALPHA_FACTOR",     rhi::Blend::AlphaFactor    },
        { "D3D12_BLEND_INV_ALPHA_FACTOR", rhi::Blend::InvAlphaFactor }
    };

    const std::map<std::string, rhi::BlendOpType> BLEND_OP =
    {
        { "D3D12_BLEND_OP_ADD",          rhi::BlendOpType::Add        },
        { "D3D12_BLEND_OP_SUBTRACT",     rhi::BlendOpType::Subtract   },
        { "D3D12_BLEND_OP_REV_SUBTRACT", rhi::BlendOpType::RevSubtract},
        { "D3D12_BLEND_OP_MIN",          rhi::BlendOpType::Min        },
        { "D3D12_BLEND_OP_MAX",          rhi::BlendOpType::Max        }
    };

    const std::map<std::string, rhi::ColorWriteEnable> COLOR_WRITE =
    {
        { "D3D12_COLOR_WRITE_DISABLE",        rhi::ColorWriteEnable::DisableAll },
        { "D3D12_COLOR_WRITE_ENABLE_RED",     rhi::ColorWriteEnable::Red        },
        { "D3D12_COLOR_WRITE_ENABLE_GREEN",   rhi::ColorWriteEnable::Green      },
        { "D3D12_COLOR_WRITE_ENABLE_BLUE",    rhi::ColorWriteEnable::Blue       },
        { "D3D12_COLOR_WRITE_ENABLE_ALPHA",   rhi::ColorWriteEnable::Alpha      },
        { "D3D12_COLOR_WRITE_ENABLE_ALL",     rhi::ColorWriteEnable::All        }
    };

    const std::map<std::string, rhi::LogicOp> LOGIC_OP =
    {
        { "D3D12_LOGIC_OP_CLEAR",        rhi::LogicOp::Clear       },
        { "D3D12_LOGIC_OP_SET",          rhi::LogicOp::Set         },
        { "D3D12_LOGIC_OP_COPY",         rhi::LogicOp::Copy        },
        { "D3D12_LOGIC_OP_COPY_INVERTED",rhi::LogicOp::CopyInverted},
        { "D3D12_LOGIC_OP_NOOP",         rhi::LogicOp::NoOp        },
        { "D3D12_LOGIC_OP_INVERT",       rhi::LogicOp::Invert      },
        { "D3D12_LOGIC_OP_AND",          rhi::LogicOp::And         },
        { "D3D12_LOGIC_OP_NAND",         rhi::LogicOp::Nand        },
        { "D3D12_LOGIC_OP_OR",           rhi::LogicOp::Or          },
        { "D3D12_LOGIC_OP_NOR",          rhi::LogicOp::Nor         },
        { "D3D12_LOGIC_OP_XOR",          rhi::LogicOp::Xor         },
        { "D3D12_LOGIC_OP_EQUIV",        rhi::LogicOp::Equiv       },
        { "D3D12_LOGIC_OP_AND_REVERSE",  rhi::LogicOp::AndReverse  },
        { "D3D12_LOGIC_OP_AND_INVERTED", rhi::LogicOp::AndInverted },
        { "D3D12_LOGIC_OP_OR_REVERSE",   rhi::LogicOp::OrReverse   },
        { "D3D12_LOGIC_OP_OR_INVERTED",  rhi::LogicOp::OrInverted  }
    };

    const std::map<std::string, rhi::FillMode> FILL_MODE =
    {
        { "D3D12_FILL_MODE_WIREFRAME", rhi::FillMode::Wireframe },
        { "D3D12_FILL_MODE_SOLID",     rhi::FillMode::Solid     }
    };

    const std::map<std::string, rhi::CullMode> CULL_MODE =
    {
        { "D3D12_CULL_MODE_NONE",  rhi::CullMode::None  },
        { "D3D12_CULL_MODE_FRONT", rhi::CullMode::Front },
        { "D3D12_CULL_MODE_BACK",  rhi::CullMode::Back  }
    };

    const std::map<std::string, rhi::ComparisonFunc> COMPARISON_FUNC =
    {
        { "D3D12_COMPARISON_FUNC_NEVER",         rhi::ComparisonFunc::Never        },
        { "D3D12_COMPARISON_FUNC_LESS",          rhi::ComparisonFunc::Less         },
        { "D3D12_COMPARISON_FUNC_EQUAL",         rhi::ComparisonFunc::Equal        },
        { "D3D12_COMPARISON_FUNC_LESS_EQUAL",    rhi::ComparisonFunc::LessEqual    },
        { "D3D12_COMPARISON_FUNC_GREATER",       rhi::ComparisonFunc::Greater      },
        { "D3D12_COMPARISON_FUNC_NOT_EQUAL",     rhi::ComparisonFunc::NotEqual     },
        { "D3D12_COMPARISON_FUNC_GREATER_EQUAL", rhi::ComparisonFunc::GreaterEqual },
        { "D3D12_COMPARISON_FUNC_ALWAYS",        rhi::ComparisonFunc::Always       }
    };

    const std::map<std::string, rhi::DepthWriteMask> DEPTH_WRITE_MASK =
    {
        { "D3D12_DEPTH_WRITE_MASK_ZERO", rhi::DepthWriteMask::Zero },
        { "D3D12_DEPTH_WRITE_MASK_ALL",  rhi::DepthWriteMask::All  }
    };

} // namespace

namespace rhi::internal
{
    PipelineStateType ParsePipelineType(const std::string& typeName)
    {
        auto it = PIPELINE_TYPE.find(typeName);
        if (it != PIPELINE_TYPE.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the pipeline type description", typeName);
        return PIPELINE_TYPE.begin()->second;
    }

    Json::Value ParseJson(const std::string& filepath)
    {
        ASSERT(std::filesystem::exists(filepath), "Failed to load {}" + filepath);

        std::ifstream file(filepath, std::ios_base::binary);
        file.open(filepath, std::ios_base::binary);
        Json::Value root;

        file >> root;

        return root;
    }

    Format ParseFormat(const std::string& str)
    {
        auto it = FORMAT.find(str);
        if (it != FORMAT.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the tech description", str);
        return FORMAT.begin()->second;
    }

    Blend ParseBlend(const std::string& str)
    {
        auto it = BLEND.find(str);
        if (it != BLEND.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the blend description", str);
        return BLEND.begin()->second;
    }

    BlendOpType ParseBlendOp(const std::string& str)
    {
        auto it = BLEND_OP.find(str);
        if (it != BLEND_OP.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the blend description", str);
        return BLEND_OP.begin()->second;
    }

    ColorWriteEnable ParseColorWriteEnable(const std::string& str)
    {
        auto it = COLOR_WRITE.find(str);
        if (it != COLOR_WRITE.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the blend description", str);
        return COLOR_WRITE.begin()->second;
    }

    LogicOp ParseLogicOp(const std::string& str)
    {
        auto it = LOGIC_OP.find(str);
        if (it != LOGIC_OP.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the blend description", str);
        return LOGIC_OP.begin()->second;
    }

    FillMode ParseFillMode(const std::string& str)
    {
        auto it = FILL_MODE.find(str);
        if (it != FILL_MODE.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the raster description", str);
        return FILL_MODE.begin()->second;
    }

    CullMode ParseCullMode(const std::string& str)
    {
        auto it = CULL_MODE.find(str);
        if (it != CULL_MODE.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the raster description", str);
        return CULL_MODE.begin()->second;
    }

    ComparisonFunc ParseComparisonFunc(const std::string& str)
    {
        auto it = COMPARISON_FUNC.find(str);
        if (it != COMPARISON_FUNC.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the depth stencil description", str);
        return COMPARISON_FUNC.begin()->second;
    }

    DepthWriteMask ParseDepthWriteMask(const std::string& str)
    {
        auto it = DEPTH_WRITE_MASK.find(str);
        if (it != DEPTH_WRITE_MASK.end())
        {
            return it->second;
        }

        LOG_WARNING("Failed to parse {} from the depth stencil description", str);
        return DEPTH_WRITE_MASK.begin()->second;
    }

} // namespace rhi::internal
