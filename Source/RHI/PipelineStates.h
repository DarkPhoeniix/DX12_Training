#pragma once

#include <stdint.h>
#include <array>

namespace rhi
{
    // Number of 32-bit values in the root constant slot (b1). Must match num32BitConstants
    // declared by URootSignature in UnifiedRootSignature.hlsli
    constexpr std::uint32_t ROOT_CONSTANT_COUNT = 16;

    // PipelineStateType represents the type of pipeline state.
    // It is used to specify the intended usage of a pipeline state object and determine the appropriate stages of the pipeline
    enum class PipelineStateType : std::uint8_t
    {
        Graphics,
        Compute
    };

    // PrimitiveTopologyType represents the type of primitive topology. 
    // It is used to specify how vertex data should be interpreted and assembled into geometric primitives for rendering
    enum class PrimitiveTopologyType : std::uint8_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip
    };

    // PrimitiveTopology represents the specific primitive topology configuration, including both basic topologies and patch list topologies for tessellation
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

    // FillMode represents the mode used for rasterization, specifying whether to render primitives as solid shapes or as wireframes
    enum class FillMode : std::uint8_t
    {
        Solid,
        Wireframe
    };

    // CullMode represents the mode used for culling, specifying which faces of primitives should be culled (not rendered) based on their orientation
    enum class CullMode : std::uint8_t
    {
        None,
        Front,
        Back
    };

    // ComparisonFunc represents the function used for depth and stencil comparisons, specifying how the GPU should compare values for operations
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

    // DepthWriteMask represents the mask used for depth writing, specifying whether depth values should be written to the depth buffer or not
    enum class DepthWriteMask : std::uint8_t
    {
        Zero,
        All
    };

    // StencilOp represents the operation used for stencil testing, specifying how the GPU should modify stencil values based on the results of stencil tests
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

    // RasterizerState represents the configuration of the rasterizer stage of the graphics pipeline, specifying how primitives should be rasterized and culled
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

    // Blend represents the blending factors used for blending operations in the output merger stage of the graphics pipeline, 
    // specifying how source and destination colors should be combined
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

    // BlendOpType represents the blending operation used for blending operations in the output merger stage of the graphics pipeline, 
    // specifying how source and destination colors should be combined based on the specified blending factors
    enum class BlendOpType
    {
        Add = 1,
        Subtract,
        RevSubtract,
        Min,
        Max
    };

    // LogicOp represents the logical operation used for blending operations in the output merger stage of the graphics pipeline,
    // specifying how source and destination colors should be combined based on a logical operation when logic operations are enabled
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

    // ColorWriteEnable represents the color channels that are enabled for writing in the output merger stage of the graphics pipeline,
    // allowing the application to specify which color components should be written to the render target during rendering operations
    enum class ColorWriteEnable
    {
        DisableAll = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = Red | Green | Blue | Alpha
    };

    // RTBlendState represents the blending state for a single render target in the output merger stage of the graphics pipeline,
    // allowing the application to specify blending factors, operations, and write masks for each render target independently
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

    // BlendState represents the overall blending state for the output merger stage of the graphics pipeline
    struct BlendState
    {
        bool AlphaToCoverageEnable;
        bool IndependentBlendEnable;
        std::array<RTBlendState, 8> RenderTargets;
    };

    // DepthStencilOpDesc represents the operations and comparison function used for stencil testing in the depth-stencil stage of the graphics pipeline,
    // allowing the application to specify how stencil values should be modified based on the results of stencil tests for both front and back faces of primitives
    struct DepthStencilOpDesc
    {
        StencilOp StencilFailOp;
        StencilOp DepthFailOp;
        StencilOp StencilPassOp;
        ComparisonFunc StencilFunc;
    };

    // DepthStencilState represents the overall depth-stencil state for the depth-stencil stage of the graphics pipeline,
    // allowing the application to specify depth testing and stencil testing configurations
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
