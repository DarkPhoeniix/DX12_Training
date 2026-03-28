#pragma once

#include "Utility/Helpers.h"

#include <stdint.h>

namespace rhi
{
    using ResourceID = std::uint64_t;
    static ResourceID InvalidResourceID = std::uint64_t(-1);

    enum class ResourceUsage : std::uint8_t
    {
        Default,
        Upload,
        GPUUpload,
        Readback
    };

    enum class ResourceFlags : std::uint8_t
    {
        None                            = 0,
        AllowRenderTarget               = 1 << 0,
        AllowDepthStencil               = 1 << 1,
        AllowUnorderedAccess            = 1 << 2,
        DenyShaderResource              = 1 << 3,
        AllowSimultaneousAccess         = 1 << 4,
        RaytracingAccelerationStructure = 1 << 5
    };
    DEFINE_ENUM_FLAG_OPERATORS(ResourceFlags);

    enum class ResourceState : std::uint16_t
    {
        Common                          = 0,
        Present                         = Common,
        VertexAndConstantBuffer         = 1 << 0,
        IndexBuffer                     = 1 << 1,
        RenderTarget                    = 1 << 2,
        UnorderedAccess                 = 1 << 3,
        DepthWrite                      = 1 << 4,
        DepthRead                       = 1 << 5,
        NonPixelShaderResource          = 1 << 6,
        PixelShaderResource             = 1 << 7,
        IndirectArgument                = 1 << 8,
        CopyDest                        = 1 << 9,
        CopySource                      = 1 << 10,
        ResolveDest                     = 1 << 11,
        ResolveSource                   = 1 << 12,
        Split                           = 1 << 13,

        GenericRead                     = VertexAndConstantBuffer | IndexBuffer | NonPixelShaderResource | PixelShaderResource | IndirectArgument | CopySource,
        AllShaderResource               = NonPixelShaderResource | PixelShaderResource,
        AllDSV                          = DepthRead | DepthWrite,
        AllCopy                         = CopySource | CopyDest,
        AllResolve                      = ResolveSource | ResolveDest,
    };
    DEFINE_ENUM_FLAG_OPERATORS(ResourceState);

    enum class TextureDimension : std::uint8_t
    {
        Unknown = 0,
        Texture1D = 1,
        Texture2D = 2,
        Texture3D = 3
    };

    enum class ResourceViewType
    {
        RTV,
        DSV,
        CBV,
        SRV,
        UAV
    };

    struct AllocationInfo
    {
        std::uint64_t SizeInBytes; // Size of the allocated resource in bytes.
        std::uint64_t Alignment;   // Alignment requirement for the resource.
    };
} // namespace rhi
