#pragma once

#include <stdint.h>
#include <array>

namespace rhi
{
    enum class PipelineStateType : std::uint8_t
    {
        Graphics,
        Compute
    };

    enum class PrimitiveTopologyType : std::uint8_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    enum class PrimitiveTopology : std::uint8_t
    {
        Undefined,
        TriangleList,
        TriangleStrip,
        PointList,
        LineList,
        LineStrip,
        PatchList1,
        PatchList2,
        PatchList3,
        PatchList4,
        PatchList5,
        PatchList6,
        PatchList7,
        PatchList8,
        PatchList9,
        PatchList10,
        PatchList11,
        PatchList12,
        PatchList13,
        PatchList14,
        PatchList15,
        PatchList16,
        PatchList17,
        PatchList18,
        PatchList19,
        PatchList20,
        PatchList21,
        PatchList22,
        PatchList23,
        PatchList24,
        PatchList25,
        PatchList26,
        PatchList27,
        PatchList28,
        PatchList29,
        PatchList30,
        PatchList31,
        PatchList32
    };

    enum class FillMode : std::uint8_t
    {
        Solid,
        Wireframe
    };

    enum class CullMode : std::uint8_t
    {
        None,
        Front,
        Back
    };

    enum class ComparisonFunc : std::uint8_t
    {
        None,
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class DepthWriteMask : std::uint8_t
    {
        Zero,
        All
    };

    enum class StencilOp : std::uint8_t
    {
        Keep,
        Zero,
        Replace,
        IncrSat,
        DecrSat,
        Invert,
        Incr,
        Decr
    };

    struct RasterizerState
    {
        FillMode FillMode = FillMode::Solid;
        CullMode CullMode = CullMode::Back;
        bool FrontCCW = false;
        int DepthBias = 0;
        float DepthBiasClamp = 0.0f;
        float SlopeScaledDepthBias = 0.0f;
        bool DepthClipEnable = true;
        bool MultisampleEnable = false;
        bool AntialiasedLineEnable = false;
        std::uint32_t ForcedSampleCount = 0;
        bool ConservativeRaster = false;
    };

    enum class Blend
    {
        Zero = 1,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha,
        AlphaFactor,
        InvAlphaFactor
    };

    enum class BlendOpType
    {
        Add = 1,
        Subtract,
        RevSubtract,
        Min,
        Max
    };

    enum class LogicOp
    {
        Clear,
        Set,
        Copy,
        CopyInverted,
        NoOp,
        Invert,
        And,
        Nand,
        Or,
        Nor,
        Xor,
        Equiv,
        AndReverse,
        AndInverted,
        OrReverse,
        OrInverted
    };

    enum class ColorWriteEnable
    {
        DisableAll = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = Red | Green | Blue | Alpha
    };

    struct RTBlendState
    {
        bool BlendEnable = false;
        bool LogicOpEnable = false;
        Blend SrcBlend = Blend::One;
        Blend DestBlend = Blend::Zero;
        BlendOpType BlendOp = BlendOpType::Add;
        Blend SrcBlendAlpha = Blend::One;
        Blend DestBlendAlpha = Blend::Zero;
        BlendOpType BlendOpAlpha = BlendOpType::Add;
        LogicOp LogicOp = LogicOp::NoOp;
        ColorWriteEnable RenderTargetWriteMask = ColorWriteEnable::All;
    };

    struct BlendState
    {
        bool AlphaToCoverageEnable;
        bool IndependentBlendEnable;
        std::array<RTBlendState, 8> RenderTargets;
    };

    struct DepthStencilOpDesc
    {
        StencilOp StencilFailOp;
        StencilOp DepthFailOp;
        StencilOp StencilPassOp;
        ComparisonFunc StencilFunc;
    };

    struct DepthStencilState
    {
        bool DepthEnable;
        DepthWriteMask DepthWriteMask;
        ComparisonFunc DepthFunc;
        bool StencilEnable;
        std::uint8_t StencilReadMask;
        std::uint8_t StencilWriteMask;
        DepthStencilOpDesc FrontFace;
        DepthStencilOpDesc BackFace;
    };
} // namespace rhi
