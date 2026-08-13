
#include "RHI_PCH.h"

#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    vk::PrimitiveTopology GetVkPrimitiveTopology(rhi::PrimitiveTopology topology)
    {
        switch (topology)
        {
        case rhi::PrimitiveTopology::PointList: return vk::PrimitiveTopology::ePointList;
        case rhi::PrimitiveTopology::LineList: return vk::PrimitiveTopology::eLineList;
        case rhi::PrimitiveTopology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
        case rhi::PrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
        case rhi::PrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
        default:
            UNREACHABLE("Unsupported primitive topology!");
            return vk::PrimitiveTopology();
        }
    }

    vk::BlendFactor GetVkBlendFactor(Blend blend)
    {
        switch (blend)
        {
        case Blend::Zero: return vk::BlendFactor::eZero;
        case Blend::One: return vk::BlendFactor::eOne;
        case Blend::SrcColor: return vk::BlendFactor::eSrcColor;
        case Blend::InvSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
        case Blend::DestColor: return vk::BlendFactor::eDstColor;
        case Blend::InvDestColor: return vk::BlendFactor::eOneMinusDstColor;
        case Blend::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
        case Blend::InvSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
        case Blend::DestAlpha: return vk::BlendFactor::eDstAlpha;
        case Blend::InvDestAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
        case Blend::BlendFactor: return vk::BlendFactor::eConstantColor;
        case Blend::InvBlendFactor: return vk::BlendFactor::eOneMinusConstantColor;
        case Blend::Src1Color: return vk::BlendFactor::eSrc1Color;
        case Blend::InvSrc1Color: return vk::BlendFactor::eOneMinusSrc1Color;
        case Blend::Src1Alpha: return vk::BlendFactor::eSrc1Alpha;
        case Blend::InvSrc1Alpha: return vk::BlendFactor::eOneMinusSrc1Alpha;
        default:
            UNREACHABLE("Unsupported blend factor!");
            return vk::BlendFactor();
        }
    }

    vk::BlendOp GetVkBlendOp(BlendOpType blendOp)
    {
        switch (blendOp)
        {
        case BlendOpType::Add: return vk::BlendOp::eAdd;
        case BlendOpType::Subtract: return vk::BlendOp::eSubtract;
        case BlendOpType::RevSubtract: return vk::BlendOp::eReverseSubtract;
        case BlendOpType::Min: return vk::BlendOp::eMin;
        case BlendOpType::Max: return vk::BlendOp::eMax;
        default:
            UNREACHABLE("Unsupported blend operation!");
            return vk::BlendOp();
        }
    }

    vk::LogicOp GetVkLogicOp(LogicOp logicOp)
    {
        switch (logicOp)
        {
        case LogicOp::Clear: return vk::LogicOp::eClear;
        case LogicOp::Set: return vk::LogicOp::eSet;
        case LogicOp::Copy: return vk::LogicOp::eCopy;
        case LogicOp::CopyInverted: return vk::LogicOp::eCopyInverted;
        case LogicOp::NoOp: return vk::LogicOp::eNoOp;
        case LogicOp::Invert: return vk::LogicOp::eInvert;
        case LogicOp::And: return vk::LogicOp::eAnd;
        case LogicOp::Nand: return vk::LogicOp::eNand;
        case LogicOp::Or: return vk::LogicOp::eOr;
        case LogicOp::Nor: return vk::LogicOp::eNor;
        case LogicOp::Xor: return vk::LogicOp::eXor;
        case LogicOp::Equiv: return vk::LogicOp::eEquivalent;
        case LogicOp::AndReverse: return vk::LogicOp::eAndReverse;
        case LogicOp::AndInverted: return vk::LogicOp::eAndInverted;
        case LogicOp::OrReverse: return vk::LogicOp::eOrReverse;
        case LogicOp::OrInverted: return vk::LogicOp::eOrInverted;
        default:
            UNREACHABLE("Unsupported logic operation!");
            return vk::LogicOp();
        }
    }

    vk::PolygonMode GetVkPolygonMode(FillMode fillMode)
    {
        switch (fillMode)
        {
        case FillMode::Solid: return vk::PolygonMode::eFill;
        case FillMode::Wireframe: return vk::PolygonMode::eLine;
        default:
            UNREACHABLE("Unsupported fill mode!");
            return vk::PolygonMode();
        }
    }

    vk::CullModeFlags GetVkCullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
        case CullMode::None: return vk::CullModeFlagBits::eNone;
        case CullMode::Front: return vk::CullModeFlagBits::eFront;
        case CullMode::Back: return vk::CullModeFlagBits::eBack;
        default:
            UNREACHABLE("Unsupported cull mode!");
            return vk::CullModeFlags();
        }
    }

    vk::FrontFace GetVkFrontFace(bool isCCW)
    {
        return isCCW ? vk::FrontFace::eCounterClockwise : vk::FrontFace::eClockwise;
    }

    vk::ColorComponentFlags GetVkColorWriteMask(ColorWriteEnable colorWriteMask)
    {
        vk::ColorComponentFlags vkMask = {};
        if (static_cast<std::uint32_t>(colorWriteMask) & static_cast<std::uint32_t>(ColorWriteEnable::Red))
            vkMask |= vk::ColorComponentFlagBits::eR;
        if (static_cast<std::uint32_t>(colorWriteMask) & static_cast<std::uint32_t>(ColorWriteEnable::Green))
            vkMask |= vk::ColorComponentFlagBits::eG;
        if (static_cast<std::uint32_t>(colorWriteMask) & static_cast<std::uint32_t>(ColorWriteEnable::Blue))
            vkMask |= vk::ColorComponentFlagBits::eB;
        if (static_cast<std::uint32_t>(colorWriteMask) & static_cast<std::uint32_t>(ColorWriteEnable::Alpha))
            vkMask |= vk::ColorComponentFlagBits::eA;
        return vkMask;
    }

    vk::PipelineRasterizationStateCreateInfo GetVkRasterizationDesc(const RasterizerState& rasterizerState)
    {
        vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo =
        {
            .flags = {},
            .depthClampEnable = vk::False,
            .rasterizerDiscardEnable = vk::False,
            .polygonMode = GetVkPolygonMode(rasterizerState.FillMode),
            .cullMode = GetVkCullMode(rasterizerState.CullMode),
            .frontFace = GetVkFrontFace(rasterizerState.FrontCCW),
            .depthBiasEnable = rasterizerState.DepthBias != 0 ? vk::True : vk::False,
            .depthBiasConstantFactor = static_cast<float>(rasterizerState.DepthBias),
            .depthBiasClamp = rasterizerState.DepthBiasClamp,
            .depthBiasSlopeFactor = rasterizerState.SlopeScaledDepthBias,
            .lineWidth = 1.0f
        };

        return rasterizationStateCreateInfo;
    }

    vk::PipelineMultisampleStateCreateInfo GetVkMultisampleDesc(const RasterizerState& multrasterizerStateisampleState)
    {
        vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo =
        {
            .flags = {},
            .rasterizationSamples = multrasterizerStateisampleState.ForcedSampleCount > 0
                ? static_cast<vk::SampleCountFlagBits>(multrasterizerStateisampleState.ForcedSampleCount)
                : vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = multrasterizerStateisampleState.MultisampleEnable,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = vk::False,
            .alphaToOneEnable = vk::False
        };
        return multisampleStateCreateInfo;
    }

    vk::PipelineDepthStencilStateCreateInfo GetVkDepthStencilDescription(const DepthStencilState& depthStencilState)
    {
        vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo =
        {
            .flags = {},
            .depthTestEnable = depthStencilState.DepthEnable,
            .depthWriteEnable = (depthStencilState.DepthWriteMask == DepthWriteMask::All) ? vk::True : vk::False,
            .depthCompareOp = GetVkCompareOp(depthStencilState.DepthFunc),
            .depthBoundsTestEnable = vk::False,
            .stencilTestEnable = depthStencilState.StencilEnable,
            .front = {
                .failOp = vk::StencilOp::eKeep,
                .passOp = vk::StencilOp::eKeep,
                .depthFailOp = vk::StencilOp::eKeep,
                .compareOp = vk::CompareOp::eAlways,
                .compareMask = depthStencilState.StencilReadMask,
                .writeMask = depthStencilState.StencilWriteMask,
                .reference = 0
            },
            .back = {
                .failOp = vk::StencilOp::eKeep,
                .passOp = vk::StencilOp::eKeep,
                .depthFailOp = vk::StencilOp::eKeep,
                .compareOp = vk::CompareOp::eAlways,
                .compareMask = depthStencilState.StencilReadMask,
                .writeMask = depthStencilState.StencilWriteMask,
                .reference = 0
            },
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
        };

        return depthStencilStateCreateInfo;
    }

    vk::PipelineColorBlendAttachmentState GetVkColorBlendAttachmentDesc(const RTBlendState& rtBlendState)
    {
        vk::PipelineColorBlendAttachmentState attachmentState =
        {
            .blendEnable = rtBlendState.BlendEnable,
            .srcColorBlendFactor = GetVkBlendFactor(rtBlendState.SrcBlend),
            .dstColorBlendFactor = GetVkBlendFactor(rtBlendState.DestBlend),
            .colorBlendOp = GetVkBlendOp(rtBlendState.BlendOp),
            .srcAlphaBlendFactor = GetVkBlendFactor(rtBlendState.SrcBlendAlpha),
            .dstAlphaBlendFactor = GetVkBlendFactor(rtBlendState.DestBlendAlpha),
            .alphaBlendOp = GetVkBlendOp(rtBlendState.BlendOpAlpha),
            .colorWriteMask = GetVkColorWriteMask(rtBlendState.RenderTargetWriteMask)
        };

        return attachmentState;
    }

    vk::PipelineColorBlendStateCreateInfo GetVkColorBlendDesc(const BlendState& blendState)
    {
        vk::PipelineColorBlendStateCreateInfo info =
        {
            .flags = {},
            .logicOpEnable = vk::False,
            .logicOp = vk::LogicOp::eNoOp,
            .attachmentCount = static_cast<std::uint32_t>(blendState.RenderTargets.size()),
            .pAttachments = nullptr,
            .blendConstants = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }
        };

        return info;
    }

    vk::Format GetVkFormat(rhi::Format format)
    {
        static const std::unordered_map<rhi::Format, vk::Format> s_FormatMap =
        {
            { rhi::Format::R32G32B32A32_TYPELESS,    vk::Format::eR32G32B32A32Sfloat    },
            { rhi::Format::R32G32B32A32_FLOAT,       vk::Format::eR32G32B32A32Sfloat    },
            { rhi::Format::R32G32B32A32_UINT,        vk::Format::eR32G32B32A32Uint      },
            { rhi::Format::R32G32B32A32_SINT,        vk::Format::eR32G32B32A32Sint      },
            { rhi::Format::R32G32B32_TYPELESS,       vk::Format::eR32G32B32Sfloat       },
            { rhi::Format::R32G32B32_FLOAT,          vk::Format::eR32G32B32Sfloat       },
            { rhi::Format::R32G32B32_UINT,           vk::Format::eR32G32B32Uint         },
            { rhi::Format::R32G32B32_SINT,           vk::Format::eR32G32B32Sint         },
            { rhi::Format::R16G16B16A16_TYPELESS,    vk::Format::eR16G16B16A16Sfloat    },
            { rhi::Format::R16G16B16A16_FLOAT,       vk::Format::eR16G16B16A16Sfloat    },
            { rhi::Format::R16G16B16A16_UNORM,       vk::Format::eR16G16B16A16Unorm     },
            { rhi::Format::R16G16B16A16_UINT,        vk::Format::eR16G16B16A16Uint      },
            { rhi::Format::R16G16B16A16_SNORM,       vk::Format::eR16G16B16A16Snorm     },
            { rhi::Format::R16G16B16A16_SINT,        vk::Format::eR16G16B16A16Sint      },
            { rhi::Format::R32G32_TYPELESS,          vk::Format::eR32G32Sfloat          },
            { rhi::Format::R32G32_FLOAT,             vk::Format::eR32G32Sfloat          },
            { rhi::Format::R32G32_UINT,              vk::Format::eR32G32Uint            },
            { rhi::Format::R32G32_SINT,              vk::Format::eR32G32Sint            },
            { rhi::Format::D32_FLOAT_S8X24_UINT,     vk::Format::eD32SfloatS8Uint       },
            { rhi::Format::R10G10B10A2_TYPELESS,     vk::Format::eA2R10G10B10UnormPack32},
            { rhi::Format::R10G10B10A2_UNORM,        vk::Format::eA2R10G10B10UnormPack32},
            { rhi::Format::R10G10B10A2_UINT,         vk::Format::eA2R10G10B10UintPack32 },
            { rhi::Format::R11G11B10_FLOAT,          vk::Format::eB10G11R11UfloatPack32 },
            { rhi::Format::R8G8B8A8_TYPELESS,        vk::Format::eR8G8B8A8Unorm         },
            { rhi::Format::R8G8B8A8_UNORM,           vk::Format::eR8G8B8A8Unorm         },
            { rhi::Format::R8G8B8A8_UNORM_SRGB,      vk::Format::eR8G8B8A8Srgb          },
            { rhi::Format::R8G8B8A8_UINT,            vk::Format::eR8G8B8A8Uint          },
            { rhi::Format::R8G8B8A8_SNORM,           vk::Format::eR8G8B8A8Snorm         },
            { rhi::Format::R8G8B8A8_SINT,            vk::Format::eR8G8B8A8Sint          },
            { rhi::Format::R16G16_TYPELESS,          vk::Format::eR16G16Sfloat          },
            { rhi::Format::R16G16_FLOAT,             vk::Format::eR16G16Sfloat          },
            { rhi::Format::R16G16_UNORM,             vk::Format::eR16G16Unorm           },
            { rhi::Format::R16G16_UINT,              vk::Format::eR16G16Uint            },
            { rhi::Format::R16G16_SNORM,             vk::Format::eR16G16Snorm           },
            { rhi::Format::R16G16_SINT,              vk::Format::eR16G16Sint            },
            { rhi::Format::R32_TYPELESS,             vk::Format::eR32Sfloat             },
            { rhi::Format::D32_FLOAT,                vk::Format::eD32Sfloat             },
            { rhi::Format::R32_FLOAT,                vk::Format::eR32Sfloat             },
            { rhi::Format::R32_UINT,                 vk::Format::eR32Uint               },
            { rhi::Format::R32_SINT,                 vk::Format::eR32Sint               },
            { rhi::Format::D24_UNORM_S8_UINT,        vk::Format::eD24UnormS8Uint        },
            { rhi::Format::R8G8_TYPELESS,            vk::Format::eR8G8Unorm             },
            { rhi::Format::R8G8_UNORM,               vk::Format::eR8G8Unorm             },
            { rhi::Format::R8G8_UINT,                vk::Format::eR8G8Uint              },
            { rhi::Format::R8G8_SNORM,               vk::Format::eR8G8Snorm             },
            { rhi::Format::R8G8_SINT,                vk::Format::eR8G8Sint              },
            { rhi::Format::R16_TYPELESS,             vk::Format::eR16Sfloat             },
            { rhi::Format::R16_FLOAT,                vk::Format::eR16Sfloat             },
            { rhi::Format::D16_UNORM,                vk::Format::eD16Unorm              },
            { rhi::Format::R16_UNORM,                vk::Format::eR16Unorm              },
            { rhi::Format::R16_UINT,                 vk::Format::eR16Uint               },
            { rhi::Format::R16_SNORM,                vk::Format::eR16Snorm              },
            { rhi::Format::R16_SINT,                 vk::Format::eR16Sint               },
            { rhi::Format::R8_TYPELESS,              vk::Format::eR8Unorm               },
            { rhi::Format::R8_UNORM,                 vk::Format::eR8Unorm               },
            { rhi::Format::R8_UINT,                  vk::Format::eR8Uint                },
            { rhi::Format::R8_SNORM,                 vk::Format::eR8Snorm               },
            { rhi::Format::R8_SINT,                  vk::Format::eR8Sint                },
            { rhi::Format::BC1_UNORM,                vk::Format::eBc1RgbaUnormBlock     },
            { rhi::Format::BC1_UNORM_SRGB,           vk::Format::eBc1RgbaSrgbBlock      },
            { rhi::Format::BC2_UNORM,                vk::Format::eBc2UnormBlock         },
            { rhi::Format::BC2_UNORM_SRGB,           vk::Format::eBc2SrgbBlock          },
            { rhi::Format::BC3_UNORM,                vk::Format::eBc3UnormBlock         },
            { rhi::Format::BC3_UNORM_SRGB,           vk::Format::eBc3SrgbBlock          },
            { rhi::Format::BC4_UNORM,                vk::Format::eBc4UnormBlock         },
            { rhi::Format::BC4_SNORM,                vk::Format::eBc4SnormBlock         },
            { rhi::Format::BC5_UNORM,                vk::Format::eBc5UnormBlock         },
            { rhi::Format::BC5_SNORM,                vk::Format::eBc5SnormBlock         },
            { rhi::Format::BC6H_UF16,                vk::Format::eBc6HUfloatBlock       },
            { rhi::Format::BC6H_SF16,                vk::Format::eBc6HSfloatBlock       },
            { rhi::Format::BC7_UNORM,                vk::Format::eBc7UnormBlock         },
            { rhi::Format::BC7_UNORM_SRGB,           vk::Format::eBc7SrgbBlock          },
            { rhi::Format::B8G8R8A8_UNORM,           vk::Format::eB8G8R8A8Unorm         },
            { rhi::Format::B8G8R8A8_UNORM_SRGB,      vk::Format::eB8G8R8A8Srgb          },
            { rhi::Format::B5G6R5_UNORM,             vk::Format::eB5G6R5UnormPack16     },
            { rhi::Format::B5G5R5A1_UNORM,           vk::Format::eB5G5R5A1UnormPack16   },
        };

        auto it = s_FormatMap.find(format);
        if (it != s_FormatMap.end())
        {
            return it->second;
        }

        LOG_WARNING("Unknown rhi::Format — returning eUndefined");
        return vk::Format::eUndefined;
    }

    vk::Format GetVkVertexFormat(rhi::Format format)
    {
        switch (format)
        {
        case rhi::Format::R32G32B32A32_FLOAT: return vk::Format::eR32G32B32A32Sfloat;
        case rhi::Format::R32G32B32A32_UINT:  return vk::Format::eR32G32B32A32Uint;
        case rhi::Format::R32G32B32_FLOAT:    return vk::Format::eR32G32B32Sfloat;
        case rhi::Format::R32G32B32_UINT:     return vk::Format::eR32G32B32Uint;
        case rhi::Format::R32G32_FLOAT:       return vk::Format::eR32G32Sfloat;
        case rhi::Format::R32G32_UINT:        return vk::Format::eR32G32Uint;
        case rhi::Format::R32_FLOAT:          return vk::Format::eR32Sfloat;
        case rhi::Format::R32_UINT:           return vk::Format::eR32Uint;
        default:
            UNREACHABLE("Unsupported vertex attribute format!");
            return vk::Format::eUndefined;
        }
    }

    std::uint32_t GetVkFormatSize(rhi::Format format)
    {
        switch (format)
        {
        case rhi::Format::R32G32B32A32_FLOAT:
        case rhi::Format::R32G32B32A32_UINT:  return 16;
        case rhi::Format::R32G32B32_FLOAT:
        case rhi::Format::R32G32B32_UINT:     return 12;
        case rhi::Format::R32G32_FLOAT:
        case rhi::Format::R32G32_UINT:        return 8;
        case rhi::Format::R32_FLOAT:
        case rhi::Format::R32_UINT:           return 4;
        default:
            UNREACHABLE("Unsupported vertex attribute format size!");
            return 0;
        }
    }

    vk::PipelineInputAssemblyStateCreateInfo GetVkInputAssemblyDesc(vk::PrimitiveTopology topology)
    {
        return
        {
            .topology               = topology,
            .primitiveRestartEnable = vk::False
        };
    }

    vk::PipelineViewportStateCreateInfo GetVkViewportDesc()
    {
        return
        {
            .viewportCount = 1,
            .pViewports    = nullptr,
            .scissorCount  = 1,
            .pScissors     = nullptr
        };
    }

    vk::ImageAspectFlags GetVkImageAspect(rhi::Format format)
    {
        switch (format)
        {
        case rhi::Format::D32_FLOAT:
        case rhi::Format::D16_UNORM:
        case rhi::Format::R32_TYPELESS:
            return vk::ImageAspectFlagBits::eDepth;
        case rhi::Format::D24_UNORM_S8_UINT:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        default:
            return vk::ImageAspectFlagBits::eColor;
        }
    }

    rhi::Format GetRHIFormat(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eR8G8B8A8Unorm: return rhi::Format::R8G8B8A8_UNORM;
        case vk::Format::eR8G8B8A8Srgb:  return rhi::Format::R8G8B8A8_UNORM_SRGB;
        case vk::Format::eB8G8R8A8Unorm: return rhi::Format::B8G8R8A8_UNORM;
        case vk::Format::eB8G8R8A8Srgb:  return rhi::Format::B8G8R8A8_UNORM_SRGB;
        default:
            LOG_WARNING("Unhandled vk::Format {} — returning UNKNOWN", vk::to_string(format));
            return rhi::Format::UNKNOWN;
        }
    }

    ResourceStateInfo GetVkResourceStateInfo(ResourceState state, bool isSwapChainImage)
    {
        // Common doubles as Present, and carries no access of its own
        if (state == ResourceState::Common)
        {
            return
            {
                isSwapChainImage ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eGeneral,
                vk::PipelineStageFlagBits2::eAllCommands,
                vk::AccessFlagBits2::eNone
            };
        }

        ResourceStateInfo info =
        {
            vk::ImageLayout::eUndefined,
            vk::PipelineStageFlagBits2::eNone,
            vk::AccessFlagBits2::eNone
        };

        // States are a bitmask, but an image can only be in one layout, so stages and access
        // accumulate while the layout is taken from the first match in priority order
        const auto has = [state](ResourceState flag) { return (state & flag) == flag; };
        const auto setLayout = [&info](vk::ImageLayout layout)
        {
            if (info.Layout == vk::ImageLayout::eUndefined)
            {
                info.Layout = layout;
            }
        };

        if (has(ResourceState::RenderTarget))
        {
            setLayout(vk::ImageLayout::eColorAttachmentOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            info.Access |= vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
        }
        if (has(ResourceState::DepthWrite))
        {
            setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
            info.Access |= vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
        }
        if (has(ResourceState::DepthRead))
        {
            setLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
            info.Access |= vk::AccessFlagBits2::eDepthStencilAttachmentRead;
        }
        if (has(ResourceState::UnorderedAccess))
        {
            setLayout(vk::ImageLayout::eGeneral);
            info.Stage |= vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader;
            info.Access |= vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite;
        }
        if (has(ResourceState::CopyDest))
        {
            setLayout(vk::ImageLayout::eTransferDstOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eAllTransfer;
            info.Access |= vk::AccessFlagBits2::eTransferWrite;
        }
        if (has(ResourceState::CopySource))
        {
            setLayout(vk::ImageLayout::eTransferSrcOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eAllTransfer;
            info.Access |= vk::AccessFlagBits2::eTransferRead;
        }
        if (has(ResourceState::ResolveDest))
        {
            setLayout(vk::ImageLayout::eTransferDstOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eResolve;
            info.Access |= vk::AccessFlagBits2::eTransferWrite;
        }
        if (has(ResourceState::ResolveSource))
        {
            setLayout(vk::ImageLayout::eTransferSrcOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eResolve;
            info.Access |= vk::AccessFlagBits2::eTransferRead;
        }
        if (has(ResourceState::PixelShaderResource))
        {
            setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eFragmentShader;
            info.Access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (has(ResourceState::NonPixelShaderResource))
        {
            setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            info.Stage |= vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eComputeShader;
            info.Access |= vk::AccessFlagBits2::eShaderSampledRead;
        }
        if (has(ResourceState::VertexAndConstantBuffer))
        {
            info.Stage |= vk::PipelineStageFlagBits2::eVertexAttributeInput | vk::PipelineStageFlagBits2::eVertexShader;
            info.Access |= vk::AccessFlagBits2::eVertexAttributeRead | vk::AccessFlagBits2::eUniformRead;
        }
        if (has(ResourceState::IndexBuffer))
        {
            info.Stage |= vk::PipelineStageFlagBits2::eIndexInput;
            info.Access |= vk::AccessFlagBits2::eIndexRead;
        }
        if (has(ResourceState::IndirectArgument))
        {
            info.Stage |= vk::PipelineStageFlagBits2::eDrawIndirect;
            info.Access |= vk::AccessFlagBits2::eIndirectCommandRead;
        }

        if (info.Stage == vk::PipelineStageFlagBits2::eNone)
        {
            UNREACHABLE("Unhandled resource state!");
            info.Stage = vk::PipelineStageFlagBits2::eAllCommands;
        }

        return info;
    }

    vk::ImageType GetVkImageType(TextureDimension dimension)
    {
        switch (dimension)
        {
        case TextureDimension::Texture1D: return vk::ImageType::e1D;
        case TextureDimension::Texture2D: return vk::ImageType::e2D;
        case TextureDimension::Texture3D: return vk::ImageType::e3D;
        default:
            UNREACHABLE("Unsupported texture dimension!");
            return vk::ImageType::e2D;
        }
    }

    vk::ImageUsageFlags GetVkImageUsage(ResourceFlags flags)
    {
        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

        if ((flags & ResourceFlags::AllowRenderTarget) == ResourceFlags::AllowRenderTarget)
        {
            usage |= vk::ImageUsageFlagBits::eColorAttachment;
        }
        if ((flags & ResourceFlags::AllowDepthStencil) == ResourceFlags::AllowDepthStencil)
        {
            usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        }
        if ((flags & ResourceFlags::AllowUnorderedAccess) == ResourceFlags::AllowUnorderedAccess)
        {
            usage |= vk::ImageUsageFlagBits::eStorage;
        }
        if ((flags & ResourceFlags::DenyShaderResource) != ResourceFlags::DenyShaderResource)
        {
            usage |= vk::ImageUsageFlagBits::eSampled;
        }

        return usage;
    }

    vk::ImageViewType GetVkImageViewType(TextureDimension dimension, std::uint32_t arraySize)
    {
        switch (dimension)
        {
        case TextureDimension::Texture1D:
            return arraySize == 0 ? vk::ImageViewType::e1D : vk::ImageViewType::e1DArray;
        case TextureDimension::Texture2D:
            // Matches the D3D12 side, where an array size of 6 is taken to mean a cube map
            if (arraySize == 6)  return vk::ImageViewType::eCube;
            return arraySize == 0 ? vk::ImageViewType::e2D : vk::ImageViewType::e2DArray;
        case TextureDimension::Texture3D:
            return vk::ImageViewType::e3D;
        default:
            UNREACHABLE("Unsupported texture dimension!");
            return vk::ImageViewType::e2D;
        }
    }

    vk::DescriptorType GetVkDescriptorType(ResourceViewType viewType)
    {
        switch (viewType)
        {
        case ResourceViewType::CBV: return vk::DescriptorType::eUniformBuffer;
        case ResourceViewType::SRV: return vk::DescriptorType::eSampledImage;
        case ResourceViewType::UAV: return vk::DescriptorType::eStorageImage;
        default:
            UNREACHABLE("View type has no descriptor representation!");
            return vk::DescriptorType::eSampledImage;
        }
    }

    vk::ImageLayout GetVkDescriptorImageLayout(ResourceViewType viewType, rhi::Format format)
    {
        if (viewType == ResourceViewType::UAV)
        {
            return vk::ImageLayout::eGeneral;
        }

        if (GetVkImageAspect(format) != vk::ImageAspectFlagBits::eColor)
        {
            return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
        }

        return vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    vk::CompareOp GetVkCompareOp(ComparisonFunc comparisonFunc)
    {
        switch (comparisonFunc)
        {
        case ComparisonFunc::None: return vk::CompareOp::eAlways; // Default to always pass if no comparison function is specified
        case ComparisonFunc::Never: return vk::CompareOp::eNever;
        case ComparisonFunc::Less: return vk::CompareOp::eLess;
        case ComparisonFunc::Equal: return vk::CompareOp::eEqual;
        case ComparisonFunc::LessEqual: return vk::CompareOp::eLessOrEqual;
        case ComparisonFunc::Greater: return vk::CompareOp::eGreater;
        case ComparisonFunc::NotEqual: return vk::CompareOp::eNotEqual;
        case ComparisonFunc::GreaterEqual: return vk::CompareOp::eGreaterOrEqual;
        case ComparisonFunc::Always: return vk::CompareOp::eAlways;
        default:
            UNREACHABLE("Unsupported comparison function!");
            return vk::CompareOp();
        }
    }
} // namespace rhi::vulkan
