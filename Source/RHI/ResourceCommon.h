#pragma once

#include "Utility/Helpers.h"

#include <cstdint>

namespace rhi
{
    // ResourceID is a unique identifier for GPU resources, used to track and manage resources within the rendering system. 
    // It allows the application to reference and manipulate resources without directly exposing the underlying resource objects, 
    // providing a level of abstraction and flexibility in resource management.
    using ResourceID = std::uint64_t;
    static ResourceID InvalidResourceID = std::uint64_t(-1);

    // ResourceUsage represents the intended usage pattern of a GPU resource, which can influence how the resource is allocated and optimized by the graphics API
    enum class ResourceUsage : std::uint8_t
    {
        Default,
        Upload,
        GPUUpload,
        Readback
    };

    // ResourceFlags represents additional flags that can be applied to a GPU resource.
    // Provides more specific information about how the resource will be used and allowing for further optimization by the graphics API
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

    // ResourceState represents the current state of a GPU resource, which is used to manage resource state transitions and ensure proper synchronization and usage of resources in rendering operations
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

    // TextureDimension represents the dimensionality of a texture resource, which can be used to specify the type of texture being created or accessed
    enum class TextureDimension : std::uint8_t
    {
        Unknown = 0,
        Texture1D = 1,
        Texture2D = 2,
        Texture3D = 3
    };

    // ResourceViewType represents the type of view that can be created for a GPU resource, 
    // which determines how the resource can be accessed and used in shaders and rendering operations
    enum class ResourceViewType
    {
        RTV,
        DSV,
        CBV,
        SRV,
        UAV
    };

    // AllocationInfo encapsulates information about a memory allocation for a GPU resource
    struct AllocationInfo
    {
        std::uint64_t SizeInBytes; // Size of the allocated resource in bytes
        std::uint64_t Alignment;   // Alignment requirement for the resource
    };
} // namespace rhi
