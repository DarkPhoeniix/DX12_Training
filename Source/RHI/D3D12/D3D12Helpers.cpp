
#include "RHI_PCH.h"

#include "D3D12Helpers.h"

#include "Buffer.h"
#include "Texture.h"
#include "CommandSignature.h"

namespace rhi::d3d12
{
    namespace
    {
        static std::unordered_map<Format, DXGI_FORMAT> s_FormatToDXGIFormat
        {
            { Format::R32G32B32A32_TYPELESS,       DXGI_FORMAT_R32G32B32A32_TYPELESS },
            { Format::R32G32B32A32_FLOAT,          DXGI_FORMAT_R32G32B32A32_FLOAT },
            { Format::R32G32B32A32_UINT,           DXGI_FORMAT_R32G32B32A32_UINT },
            { Format::R32G32B32A32_SINT,           DXGI_FORMAT_R32G32B32A32_SINT },
            { Format::R32G32B32_TYPELESS,          DXGI_FORMAT_R32G32B32_TYPELESS },
            { Format::R32G32B32_FLOAT,             DXGI_FORMAT_R32G32B32_FLOAT },
            { Format::R32G32B32_UINT,              DXGI_FORMAT_R32G32B32_UINT },
            { Format::R32G32B32_SINT,              DXGI_FORMAT_R32G32B32_SINT },
            { Format::R16G16B16A16_TYPELESS,       DXGI_FORMAT_R16G16B16A16_TYPELESS },
            { Format::R16G16B16A16_FLOAT,          DXGI_FORMAT_R16G16B16A16_FLOAT },
            { Format::R16G16B16A16_UNORM,          DXGI_FORMAT_R16G16B16A16_UNORM },
            { Format::R16G16B16A16_UINT,           DXGI_FORMAT_R16G16B16A16_UINT },
            { Format::R16G16B16A16_SNORM,          DXGI_FORMAT_R16G16B16A16_SNORM },
            { Format::R16G16B16A16_SINT,           DXGI_FORMAT_R16G16B16A16_SINT },
            { Format::R32G32_TYPELESS,             DXGI_FORMAT_R32G32_TYPELESS },
            { Format::R32G32_FLOAT,                DXGI_FORMAT_R32G32_FLOAT },
            { Format::R32G32_UINT,                 DXGI_FORMAT_R32G32_UINT },
            { Format::R32G32_SINT,                 DXGI_FORMAT_R32G32_SINT },
            { Format::R32G8X24_TYPELESS,           DXGI_FORMAT_R32G8X24_TYPELESS },
            { Format::D32_FLOAT_S8X24_UINT,        DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
            { Format::R32_FLOAT_X8X24_TYPELESS,    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
            { Format::X32_TYPELESS_G8X24_UINT,     DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
            { Format::R10G10B10A2_TYPELESS,        DXGI_FORMAT_R10G10B10A2_TYPELESS },
            { Format::R10G10B10A2_UNORM,           DXGI_FORMAT_R10G10B10A2_UNORM },
            { Format::R10G10B10A2_UINT,            DXGI_FORMAT_R10G10B10A2_UINT },
            { Format::R11G11B10_FLOAT,             DXGI_FORMAT_R11G11B10_FLOAT },
            { Format::R8G8B8A8_TYPELESS,           DXGI_FORMAT_R8G8B8A8_TYPELESS },
            { Format::R8G8B8A8_UNORM,              DXGI_FORMAT_R8G8B8A8_UNORM },
            { Format::R8G8B8A8_UNORM_SRGB,         DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
            { Format::R8G8B8A8_UINT,               DXGI_FORMAT_R8G8B8A8_UINT },
            { Format::R8G8B8A8_SNORM,              DXGI_FORMAT_R8G8B8A8_SNORM },
            { Format::R8G8B8A8_SINT,               DXGI_FORMAT_R8G8B8A8_SINT },
            { Format::R16G16_TYPELESS,             DXGI_FORMAT_R16G16_TYPELESS },
            { Format::R16G16_FLOAT,                DXGI_FORMAT_R16G16_FLOAT },
            { Format::R16G16_UNORM,                DXGI_FORMAT_R16G16_UNORM },
            { Format::R16G16_UINT,                 DXGI_FORMAT_R16G16_UINT },
            { Format::R16G16_SNORM,                DXGI_FORMAT_R16G16_SNORM },
            { Format::R16G16_SINT,                 DXGI_FORMAT_R16G16_SINT },
            { Format::R32_TYPELESS,                DXGI_FORMAT_R32_TYPELESS },
            { Format::D32_FLOAT,                   DXGI_FORMAT_D32_FLOAT },
            { Format::R32_FLOAT,                   DXGI_FORMAT_R32_FLOAT },
            { Format::R32_UINT,                    DXGI_FORMAT_R32_UINT },
            { Format::R32_SINT,                    DXGI_FORMAT_R32_SINT },
            { Format::R24G8_TYPELESS,              DXGI_FORMAT_R24G8_TYPELESS },
            { Format::D24_UNORM_S8_UINT,           DXGI_FORMAT_D24_UNORM_S8_UINT },
            { Format::R24_UNORM_X8_TYPELESS,       DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
            { Format::X24_TYPELESS_G8_UINT,        DXGI_FORMAT_X24_TYPELESS_G8_UINT },
            { Format::R8G8_TYPELESS,               DXGI_FORMAT_R8G8_TYPELESS },
            { Format::R8G8_UNORM,                  DXGI_FORMAT_R8G8_UNORM },
            { Format::R8G8_UINT,                   DXGI_FORMAT_R8G8_UINT },
            { Format::R8G8_SNORM,                  DXGI_FORMAT_R8G8_SNORM },
            { Format::R8G8_SINT,                   DXGI_FORMAT_R8G8_SINT },
            { Format::R16_TYPELESS,                DXGI_FORMAT_R16_TYPELESS },
            { Format::R16_FLOAT,                   DXGI_FORMAT_R16_FLOAT },
            { Format::D16_UNORM,                   DXGI_FORMAT_D16_UNORM },
            { Format::R16_UNORM,                   DXGI_FORMAT_R16_UNORM },
            { Format::R16_UINT,                    DXGI_FORMAT_R16_UINT },
            { Format::R16_SNORM,                   DXGI_FORMAT_R16_SNORM },
            { Format::R16_SINT,                    DXGI_FORMAT_R16_SINT },
            { Format::R8_TYPELESS,                 DXGI_FORMAT_R8_TYPELESS },
            { Format::R8_UNORM,                    DXGI_FORMAT_R8_UNORM },
            { Format::R8_UINT,                     DXGI_FORMAT_R8_UINT },
            { Format::R8_SNORM,                    DXGI_FORMAT_R8_SNORM },
            { Format::R8_SINT,                     DXGI_FORMAT_R8_SINT },
            { Format::A8_UNORM,                    DXGI_FORMAT_A8_UNORM },
            { Format::R1_UNORM,                    DXGI_FORMAT_R1_UNORM },
            { Format::R9G9B9E5_SHAREDEXP,          DXGI_FORMAT_R9G9B9E5_SHAREDEXP },
            { Format::R8G8_B8G8_UNORM,             DXGI_FORMAT_R8G8_B8G8_UNORM },
            { Format::G8R8_G8B8_UNORM,             DXGI_FORMAT_G8R8_G8B8_UNORM },
            { Format::BC1_TYPELESS,                DXGI_FORMAT_BC1_TYPELESS },
            { Format::BC1_UNORM,                   DXGI_FORMAT_BC1_UNORM },
            { Format::BC1_UNORM_SRGB,              DXGI_FORMAT_BC1_UNORM_SRGB },
            { Format::BC2_TYPELESS,                DXGI_FORMAT_BC2_TYPELESS },
            { Format::BC2_UNORM,                   DXGI_FORMAT_BC2_UNORM },
            { Format::BC2_UNORM_SRGB,              DXGI_FORMAT_BC2_UNORM_SRGB },
            { Format::BC3_TYPELESS,                DXGI_FORMAT_BC3_TYPELESS },
            { Format::BC3_UNORM,                   DXGI_FORMAT_BC3_UNORM },
            { Format::BC3_UNORM_SRGB,              DXGI_FORMAT_BC3_UNORM_SRGB },
            { Format::BC4_TYPELESS,                DXGI_FORMAT_BC4_TYPELESS },
            { Format::BC4_UNORM,                   DXGI_FORMAT_BC4_UNORM },
            { Format::BC4_SNORM,                   DXGI_FORMAT_BC4_SNORM },
            { Format::BC5_TYPELESS,                DXGI_FORMAT_BC5_TYPELESS },
            { Format::BC5_UNORM,                   DXGI_FORMAT_BC5_UNORM },
            { Format::BC5_SNORM,                   DXGI_FORMAT_BC5_SNORM },
            { Format::B5G6R5_UNORM,                DXGI_FORMAT_B5G6R5_UNORM },
            { Format::B5G5R5A1_UNORM,              DXGI_FORMAT_B5G5R5A1_UNORM },
            { Format::B8G8R8A8_UNORM,              DXGI_FORMAT_B8G8R8A8_UNORM },
            { Format::B8G8R8X8_UNORM,              DXGI_FORMAT_B8G8R8X8_UNORM },
            { Format::R10G10B10_XR_BIAS_A2_UNORM,  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { Format::B8G8R8A8_TYPELESS,           DXGI_FORMAT_B8G8R8A8_TYPELESS },
            { Format::B8G8R8A8_UNORM_SRGB,         DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
            { Format::B8G8R8X8_TYPELESS,           DXGI_FORMAT_B8G8R8X8_TYPELESS },
            { Format::B8G8R8X8_UNORM_SRGB,         DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },
            { Format::BC6H_TYPELESS,               DXGI_FORMAT_BC6H_TYPELESS },
            { Format::BC6H_UF16,                   DXGI_FORMAT_BC6H_UF16 },
            { Format::BC6H_SF16,                   DXGI_FORMAT_BC6H_SF16 },
            { Format::BC7_TYPELESS,                DXGI_FORMAT_BC7_TYPELESS },
            { Format::BC7_UNORM,                   DXGI_FORMAT_BC7_UNORM },
            { Format::BC7_UNORM_SRGB,              DXGI_FORMAT_BC7_UNORM_SRGB }
        };

        static std::unordered_map<ResourceFlags, D3D12_RESOURCE_FLAGS> s_ResourceFlagsToD3D12ResourceFlags
        {
            { ResourceFlags::None,                             D3D12_RESOURCE_FLAG_NONE },
            { ResourceFlags::AllowRenderTarget,                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET },
            { ResourceFlags::AllowDepthStencil,                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL },
            { ResourceFlags::AllowUnorderedAccess,             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS },
            { ResourceFlags::DenyShaderResource,               D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE },
            { ResourceFlags::AllowSimultaneousAccess,          D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS },
            { ResourceFlags::RaytracingAccelerationStructure,  D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE }
        };
    } // namespace unnamed

    DXGI_FORMAT GetDXGIFormat(Format format)
    {
        auto it = s_FormatToDXGIFormat.find(format);

        if (it != s_FormatToDXGIFormat.end())
        {
            return it->second;
        }
        else
        {
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(ResourceFlags flags)
    {
        D3D12_RESOURCE_FLAGS d3d12Flags = D3D12_RESOURCE_FLAG_NONE;

        for (const auto& pair : s_ResourceFlagsToD3D12ResourceFlags)
        {
            if ((flags & pair.first) == pair.first)
            {
                d3d12Flags |= pair.second;
            }
        }

        return d3d12Flags;
    }

    D3D12_RESOURCE_DIMENSION GetD3D12ResourceDimension(TextureDimension dimension)
    {
        switch (dimension)
        {
        case TextureDimension::Unknown: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        case TextureDimension::Texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureDimension::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureDimension::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            UNREACHABLE("Unsupported texture dimension for D3D12 resource creation.");
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }

    D3D12_RESOURCE_STATES GetD3D12ResourceState(ResourceState state)
    {
        D3D12_RESOURCE_STATES d3dState = D3D12_RESOURCE_STATE_COMMON;

        if (HasFlag(state, ResourceState::Common))                  d3dState |= D3D12_RESOURCE_STATE_COMMON;
        if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) d3dState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (HasFlag(state, ResourceState::IndexBuffer))             d3dState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (HasFlag(state, ResourceState::RenderTarget))            d3dState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         d3dState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              d3dState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               d3dState |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (HasFlag(state, ResourceState::AllShaderResource))    d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::NonPixelShaderResource))  d3dState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::PixelShaderResource))     d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                d3dState |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              d3dState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (HasFlag(state, ResourceState::ResolveDest))             d3dState |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           d3dState |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        if (HasFlag(state, ResourceState::IndirectArgument))        d3dState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (HasFlag(state, ResourceState::Present))                 d3dState |= D3D12_RESOURCE_STATE_PRESENT;

        return d3dState;
    }

    D3D12_BARRIER_SYNC GetD3D12SyncFlags(ResourceState state)
    {
        D3D12_BARRIER_SYNC flags = D3D12_BARRIER_SYNC_NONE;

        if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        //if (HasFlag(state, ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_SYNC_INDEX_INPUT;
        if (HasFlag(state, ResourceState::RenderTarget))            flags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        if (HasAnyFlag(state, ResourceState::AllDSV))               flags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        if (HasFlag(state, ResourceState::NonPixelShaderResource))  flags |= D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
        if (HasFlag(state, ResourceState::PixelShaderResource))     flags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
        if (HasAnyFlag(state, ResourceState::AllCopy))              flags |= D3D12_BARRIER_SYNC_COPY;
        if (HasAnyFlag(state, ResourceState::AllResolve))           flags |= D3D12_BARRIER_SYNC_RESOLVE;
        if (HasFlag(state, ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
        if (HasFlag(state, ResourceState::Split))                   flags |= D3D12_BARRIER_SYNC_SPLIT;
        if (flags == D3D12_BARRIER_SYNC_NONE)                       flags = D3D12_BARRIER_SYNC_ALL; // Fallback to ALL if no specific sync is set

        return flags;
    }

    D3D12_BARRIER_ACCESS GetD3D12AccessFlags(ResourceState state)
    {
        D3D12_BARRIER_ACCESS flags = D3D12_BARRIER_ACCESS_COMMON;

        if (HasFlag(state, ResourceState::Common))                  flags |= D3D12_BARRIER_ACCESS_COMMON;
        //if (HasFlag(state, ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
        //if (HasFlag(state, ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
        if (HasFlag(state, ResourceState::RenderTarget))            flags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, ResourceState::AllShaderResource))    flags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                flags |= D3D12_BARRIER_ACCESS_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              flags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
        if (HasFlag(state, ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
        if (HasFlag(state, ResourceState::ResolveDest))             flags |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           flags |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;

        return flags;
    }

    D3D12_BARRIER_LAYOUT GetD3D12Layout(ResourceState state)
    {
        // TODO: it would be better to use queue-specific layouts here

        if (HasFlag(state, ResourceState::RenderTarget))            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        if (HasFlag(state, ResourceState::UnorderedAccess))         return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
        if (HasFlag(state, ResourceState::DepthWrite))              return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, ResourceState::DepthRead))               return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, ResourceState::AllShaderResource))    return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        if (HasFlag(state, ResourceState::CopyDest))                return D3D12_BARRIER_LAYOUT_COPY_DEST;
        if (HasFlag(state, ResourceState::CopySource))              return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        if (HasFlag(state, ResourceState::ResolveDest))             return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
        if (HasFlag(state, ResourceState::ResolveSource))           return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
        if (HasFlag(state, ResourceState::Present))                 return D3D12_BARRIER_LAYOUT_PRESENT;

        UNREACHABLE("Unsupported resource state for layout conversion.");
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }

    D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandListType type)
    {
        switch (type)
        {
        case CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case CommandListType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
            UNREACHABLE("Unsupported command list type for D3D12 command list creation.");
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    D3D12_PREDICATION_OP GetD3D12PredicationOp(PredicationOperation op)
    {
        switch (op)
        {
        case PredicationOperation::EqualZero: return D3D12_PREDICATION_OP_EQUAL_ZERO;
        case PredicationOperation::NotEqualZero: return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;
        default:
            UNREACHABLE("Unsupported predication operation for D3D12 command list predication.");
            return D3D12_PREDICATION_OP_EQUAL_ZERO;
        }
    }

    D3D12_QUERY_TYPE GetD3D12QueryType(QueryType type)
    {
        switch (type)
        {
        case QueryType::Occlusion: return D3D12_QUERY_TYPE_OCCLUSION;
        //case QueryType::BinaryOcclusion: return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
        case QueryType::Timestamp: return D3D12_QUERY_TYPE_TIMESTAMP;
        case QueryType::PipelineStatistics: return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
        default:
            UNREACHABLE("Unsupported query type for D3D12 command list query operations.");
            return D3D12_QUERY_TYPE_OCCLUSION;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(PrimitiveTopology primitiveTopology)
    {
        switch (primitiveTopology)
        {
        case PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::PatchList1: return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList2: return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList3: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList4: return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList5: return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList6: return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList7: return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList8: return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList9: return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList10: return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList11: return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList12: return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList13: return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList14: return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList15: return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList16: return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList17: return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList18: return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList19: return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList20: return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList21: return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList22: return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList23: return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList24: return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList25: return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList26: return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList27: return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList28: return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList29: return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList30: return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList31: return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::PatchList32: return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
        default:
            UNREACHABLE("Unsupported primitive topology.");
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    D3D12_QUERY_HEAP_TYPE GetD3D12QueryHeapType(QueryHeapType type)
    {
        switch (type)
        {
        case QueryHeapType::Occlusion: return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        case QueryHeapType::Timestamp: return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        case QueryHeapType::PipelineStatistics: return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        case QueryHeapType::PipelineStatistics1: return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS1;
        default:
            UNREACHABLE("Unsupported query heap type.");
            return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        }
    }

    D3D12_BLEND GetD3D12Blend(Blend blend)
    {
        switch (blend)
        {
        case Blend::Zero: return D3D12_BLEND_ZERO;
        case Blend::One: return D3D12_BLEND_ONE;
        case Blend::SrcColor: return D3D12_BLEND_SRC_COLOR;
        case Blend::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
        case Blend::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
        case Blend::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
        case Blend::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
        case Blend::InvDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
        case Blend::DestColor: return D3D12_BLEND_DEST_COLOR;
        case Blend::InvDestColor: return D3D12_BLEND_INV_DEST_COLOR;
        case Blend::SrcAlphaSat: return D3D12_BLEND_SRC_ALPHA_SAT;
        case Blend::BlendFactor: return D3D12_BLEND_BLEND_FACTOR;
        case Blend::InvBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
        case Blend::Src1Color: return D3D12_BLEND_SRC1_COLOR;
        case Blend::InvSrc1Color: return D3D12_BLEND_INV_SRC1_COLOR;
        case Blend::Src1Alpha: return D3D12_BLEND_SRC1_ALPHA;
        case Blend::InvSrc1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
        case Blend::AlphaFactor: return D3D12_BLEND_ALPHA_FACTOR;
        case Blend::InvAlphaFactor: return D3D12_BLEND_INV_ALPHA_FACTOR;
        default:
            UNREACHABLE("Unsupported blend type.");
            return D3D12_BLEND_ZERO;
        }
    }

    D3D12_BLEND_OP GetD3D12BlendOp(BlendOpType blendOp)
    {
        switch (blendOp)
        {
        case BlendOpType::Add: return D3D12_BLEND_OP_ADD;
        case BlendOpType::Subtract: return D3D12_BLEND_OP_SUBTRACT;
        case BlendOpType::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOpType::Min: return D3D12_BLEND_OP_MIN;
        case BlendOpType::Max: return D3D12_BLEND_OP_MAX;
        default:
            UNREACHABLE("Unsupported blend operation.");
            return D3D12_BLEND_OP_ADD;
        }
    }

    D3D12_LOGIC_OP GetD3D12LogicOp(LogicOp logicOp)
    {
        switch (logicOp)
        {
        case LogicOp::Clear: return D3D12_LOGIC_OP_CLEAR;
        case LogicOp::Set: return D3D12_LOGIC_OP_SET;
        case LogicOp::Copy: return D3D12_LOGIC_OP_COPY;
        case LogicOp::CopyInverted: return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp::NoOp: return D3D12_LOGIC_OP_NOOP;
        case LogicOp::Invert: return D3D12_LOGIC_OP_INVERT;
        case LogicOp::And: return D3D12_LOGIC_OP_AND;
        case LogicOp::Nand: return D3D12_LOGIC_OP_NAND;
        case LogicOp::Or: return D3D12_LOGIC_OP_OR;
        case LogicOp::Nor: return D3D12_LOGIC_OP_NOR;
        case LogicOp::Xor: return D3D12_LOGIC_OP_XOR;
        case LogicOp::Equiv: return D3D12_LOGIC_OP_EQUIV;
        case LogicOp::AndReverse: return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp::AndInverted: return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp::OrReverse: return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp::OrInverted: return D3D12_LOGIC_OP_OR_INVERTED;
        default:
            UNREACHABLE("Unsupported logic operation.");
            return D3D12_LOGIC_OP_CLEAR;
        }
    }

    D3D12_COLOR_WRITE_ENABLE GetD3D12RenderTargetWriteMask(ColorWriteEnable colorWriteEnable)
    {
        switch (colorWriteEnable)
        {
        case ColorWriteEnable::DisableAll: return (D3D12_COLOR_WRITE_ENABLE)0;
        case ColorWriteEnable::Red: return D3D12_COLOR_WRITE_ENABLE_RED;
        case ColorWriteEnable::Green: return D3D12_COLOR_WRITE_ENABLE_GREEN;
        case ColorWriteEnable::Blue: return D3D12_COLOR_WRITE_ENABLE_BLUE;
        case ColorWriteEnable::Alpha: return D3D12_COLOR_WRITE_ENABLE_ALPHA;
        case ColorWriteEnable::All: return D3D12_COLOR_WRITE_ENABLE_ALL;
        default:
            UNREACHABLE("Unsupported color write mask");
            return (D3D12_COLOR_WRITE_ENABLE)0;
        }
    }

    D3D12_BLEND_DESC GetD3D12BlendDesc(const BlendState& blendState)
    {
        D3D12_BLEND_DESC desc =
        {
            .AlphaToCoverageEnable = blendState.AlphaToCoverageEnable,
            .IndependentBlendEnable = blendState.IndependentBlendEnable
        };

        for (size_t i = 0; i < blendState.RenderTargets.size(); ++i)
        {
            desc.RenderTarget[i] = GetD3D12RTBlendState(blendState.RenderTargets[i]);
        }

        return desc;
    }

    D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RTBlendState(const RTBlendState& blendState)
    {
        D3D12_RENDER_TARGET_BLEND_DESC desc =
        {
            .BlendEnable = blendState.BlendEnable,
            .LogicOpEnable = blendState.LogicOpEnable,
            .SrcBlend = GetD3D12Blend(blendState.SrcBlend),
            .DestBlend = GetD3D12Blend(blendState.DestBlend),
            .BlendOp = GetD3D12BlendOp(blendState.BlendOp),
            .SrcBlendAlpha = GetD3D12Blend(blendState.SrcBlendAlpha),
            .DestBlendAlpha = GetD3D12Blend(blendState.DestBlendAlpha),
            .BlendOpAlpha = GetD3D12BlendOp(blendState.BlendOpAlpha),
            .LogicOp = GetD3D12LogicOp(blendState.LogicOp),
            .RenderTargetWriteMask = (UINT8)GetD3D12RenderTargetWriteMask(blendState.RenderTargetWriteMask)
        };

        return desc;
    }

    D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode)
    {
        switch (fillMode)
        {
        case FillMode::Solid: return D3D12_FILL_MODE_SOLID;
        case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
        default:
            UNREACHABLE("Unsupported fill mode.");
            return D3D12_FILL_MODE_SOLID;
        }
    }

    D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode)
    {
        switch (cullMode)
        {
        case CullMode::None: return D3D12_CULL_MODE_NONE;
        case CullMode::Back: return D3D12_CULL_MODE_BACK;
        case CullMode::Front: return D3D12_CULL_MODE_FRONT;
        default:
            UNREACHABLE("Unsupported cull mode.");
            return D3D12_CULL_MODE_NONE;
        }
    }

    D3D12_RASTERIZER_DESC GetD3D12RasterizerDesc(const RasterizerState& rasterizerState)
    {
        D3D12_RASTERIZER_DESC desc =
        {
            .FillMode = GetD3D12FillMode(rasterizerState.FillMode),
            .CullMode = GetD3D12CullMode(rasterizerState.CullMode),
            .FrontCounterClockwise = rasterizerState.FrontCCW,
            .DepthBias = rasterizerState.DepthBias,
            .DepthBiasClamp = rasterizerState.DepthBiasClamp,
            .SlopeScaledDepthBias = rasterizerState.SlopeScaledDepthBias,
            .DepthClipEnable = rasterizerState.DepthClipEnable,
            .MultisampleEnable = rasterizerState.MultisampleEnable,
            .AntialiasedLineEnable = rasterizerState.AntialiasedLineEnable,
            .ForcedSampleCount = rasterizerState.ForcedSampleCount,
            .ConservativeRaster = rasterizerState.ConservativeRaster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
        };

        return desc;
    }

    D3D12_DEPTH_WRITE_MASK GetD3D12DepthWriteMask(DepthWriteMask mask)
    {
        switch (mask)
        {
        case DepthWriteMask::Zero: return D3D12_DEPTH_WRITE_MASK_ZERO;
        case DepthWriteMask::All: return D3D12_DEPTH_WRITE_MASK_ALL;
        default:
            UNREACHABLE("Unsupported depth write mask.");
            return D3D12_DEPTH_WRITE_MASK_ZERO;
        }
    }

    D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(ComparisonFunc comparisonFunc)
    {
        switch (comparisonFunc)
        {
        case ComparisonFunc::None: return D3D12_COMPARISON_FUNC_NONE;
        case ComparisonFunc::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case ComparisonFunc::Less: return D3D12_COMPARISON_FUNC_LESS;
        case ComparisonFunc::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case ComparisonFunc::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case ComparisonFunc::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case ComparisonFunc::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case ComparisonFunc::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case ComparisonFunc::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            UNREACHABLE("Unsupported comparison function.");
            return D3D12_COMPARISON_FUNC_NONE;
        }
    }

    D3D12_STENCIL_OP GetD3D12StencilOp(StencilOp stencilOp)
    {
        switch (stencilOp)
        {
            case StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
            case StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
            case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
            case StencilOp::IncrSat: return D3D12_STENCIL_OP_INCR_SAT;
            case StencilOp::DecrSat: return D3D12_STENCIL_OP_DECR_SAT;
            case StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
            case StencilOp::Incr: return D3D12_STENCIL_OP_INCR;
            case StencilOp::Decr: return D3D12_STENCIL_OP_DECR;
            default:
                UNREACHABLE("Unsupported stencil op.");
                return D3D12_STENCIL_OP_ZERO;
        }
    }

    D3D12_DEPTH_STENCILOP_DESC GetD3D12StencilOpDesc(const DepthStencilOpDesc& depthStencilOpDesc)
    {
        D3D12_DEPTH_STENCILOP_DESC desc =
        {
            .StencilFailOp = GetD3D12StencilOp(depthStencilOpDesc.StencilFailOp),
            .StencilDepthFailOp = GetD3D12StencilOp(depthStencilOpDesc.DepthFailOp),
            .StencilPassOp = GetD3D12StencilOp(depthStencilOpDesc.StencilPassOp),
            .StencilFunc = GetD3D12ComparisonFunc(depthStencilOpDesc.StencilFunc)
        };

        return desc;
    }

    D3D12_DEPTH_STENCIL_DESC GetD3D12DepthStencilDesc(const DepthStencilState depthStencilState)
    {
        D3D12_DEPTH_STENCIL_DESC desc =
        {
            .DepthEnable = depthStencilState.DepthEnable,
            .DepthWriteMask = GetD3D12DepthWriteMask(depthStencilState.DepthWriteMask),
            .DepthFunc = GetD3D12ComparisonFunc(depthStencilState.DepthFunc),
            .StencilEnable = depthStencilState.StencilEnable,
            .StencilReadMask = depthStencilState.StencilReadMask,
            .StencilWriteMask = depthStencilState.StencilWriteMask,
            .FrontFace = GetD3D12StencilOpDesc(depthStencilState.FrontFace),
            .BackFace = GetD3D12StencilOpDesc(depthStencilState.BackFace)
        };

        return desc;
    }

    D3D12_CLEAR_FLAGS GetD3D12ClearFlags(ClearFlags clearFlags)
    {
        switch (clearFlags)
        {
        case ClearFlags::DepthStencil: return D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
        case ClearFlags::Depth: return D3D12_CLEAR_FLAG_DEPTH;
        case ClearFlags::Stencil: return D3D12_CLEAR_FLAG_STENCIL;
        default:
            UNREACHABLE("Unsupported clear flag.");
            return D3D12_CLEAR_FLAG_DEPTH;
        }
    }

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const BufferDescription& description)
    {
        D3D12_RESOURCE_DESC resourceDesc =
        {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = description.Size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = GetDXGIFormat(description.Format),
            .SampleDesc = { 1, 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = GetD3D12ResourceFlags(description.Flags)
        };

        return resourceDesc;
    }

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDescription& description)
    {
        D3D12_RESOURCE_DESC resourceDesc =
        {
            .Dimension = GetD3D12ResourceDimension(description.Dimension),
            .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            .Width = description.Width,
            .Height = description.Height,
            .DepthOrArraySize = description.DepthOrArraySize,
            .MipLevels = description.MipLevels,
            .Format = GetDXGIFormat(description.Format),
            .SampleDesc = { 1, 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = GetD3D12ResourceFlags(description.Flags)
        };

        return resourceDesc;
    }

    D3D12_INDIRECT_ARGUMENT_DESC GetD3D12IndirectArgumentDesc(const IndirectArgumentDescription argumentDesc)
    {
        D3D12_INDIRECT_ARGUMENT_DESC argument = {};

        switch (argumentDesc.Type)
        {
        case IndirectArgumentType::Draw:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
            break;
        case IndirectArgumentType::DrawIndexed:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
            break;
        case IndirectArgumentType::Dispatch:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
            break;
        case IndirectArgumentType::VertexBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argument.VertexBuffer.Slot = argumentDesc.VertexBuffer.Slot;
            break;
        case IndirectArgumentType::IndexBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
            break;
        case IndirectArgumentType::Constant:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            argument.Constant =
            {
                .RootParameterIndex = argumentDesc.Constant.RootParameterIndex,
                .DestOffsetIn32BitValues = argumentDesc.Constant.DestOffsetIn32BitValues,
                .Num32BitValuesToSet = argumentDesc.Constant.Num32BitValuesToSet
            };
            break;
        case IndirectArgumentType::ConstantBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argument.ConstantBufferView.RootParameterIndex = argumentDesc.ConstantBufferView.RootParameterIndex;
            break;
        case IndirectArgumentType::ShaderResourceView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argument.ShaderResourceView.RootParameterIndex = argumentDesc.ShaderResourceView.RootParameterIndex;
            break;
        case IndirectArgumentType::UnorderedResourceView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW;
            argument.UnorderedAccessView.RootParameterIndex = argumentDesc.UnorderedAccessView.RootParameterIndex;
            break;
        default:
            UNREACHABLE("Unsupported indirect argument type.");
            break;
        }

        return argument;
    }
} // namespace rhi::d3d12
