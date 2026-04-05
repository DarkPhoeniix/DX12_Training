
#include "RHI_PCH.h"

#include "D3D12Helpers.h"

#include "Buffer.h"
#include "Texture.h"
#include "CommandSignature.h"

namespace rhi::d3d12
{
    namespace
    {
        static std::unordered_map<rhi::Format, DXGI_FORMAT> s_FormatToDXGIFormat
        {
            { rhi::Format::R32G32B32A32_TYPELESS,       DXGI_FORMAT_R32G32B32A32_TYPELESS },
            { rhi::Format::R32G32B32A32_FLOAT,          DXGI_FORMAT_R32G32B32A32_FLOAT },
            { rhi::Format::R32G32B32A32_UINT,           DXGI_FORMAT_R32G32B32A32_UINT },
            { rhi::Format::R32G32B32A32_SINT,           DXGI_FORMAT_R32G32B32A32_SINT },
            { rhi::Format::R32G32B32_TYPELESS,          DXGI_FORMAT_R32G32B32_TYPELESS },
            { rhi::Format::R32G32B32_FLOAT,             DXGI_FORMAT_R32G32B32_FLOAT },
            { rhi::Format::R32G32B32_UINT,              DXGI_FORMAT_R32G32B32_UINT },
            { rhi::Format::R32G32B32_SINT,              DXGI_FORMAT_R32G32B32_SINT },
            { rhi::Format::R16G16B16A16_TYPELESS,       DXGI_FORMAT_R16G16B16A16_TYPELESS },
            { rhi::Format::R16G16B16A16_FLOAT,          DXGI_FORMAT_R16G16B16A16_FLOAT },
            { rhi::Format::R16G16B16A16_UNORM,          DXGI_FORMAT_R16G16B16A16_UNORM },
            { rhi::Format::R16G16B16A16_UINT,           DXGI_FORMAT_R16G16B16A16_UINT },
            { rhi::Format::R16G16B16A16_SNORM,          DXGI_FORMAT_R16G16B16A16_SNORM },
            { rhi::Format::R16G16B16A16_SINT,           DXGI_FORMAT_R16G16B16A16_SINT },
            { rhi::Format::R32G32_TYPELESS,             DXGI_FORMAT_R32G32_TYPELESS },
            { rhi::Format::R32G32_FLOAT,                DXGI_FORMAT_R32G32_FLOAT },
            { rhi::Format::R32G32_UINT,                 DXGI_FORMAT_R32G32_UINT },
            { rhi::Format::R32G32_SINT,                 DXGI_FORMAT_R32G32_SINT },
            { rhi::Format::R32G8X24_TYPELESS,           DXGI_FORMAT_R32G8X24_TYPELESS },
            { rhi::Format::D32_FLOAT_S8X24_UINT,        DXGI_FORMAT_D32_FLOAT_S8X24_UINT },
            { rhi::Format::R32_FLOAT_X8X24_TYPELESS,    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS },
            { rhi::Format::X32_TYPELESS_G8X24_UINT,     DXGI_FORMAT_X32_TYPELESS_G8X24_UINT },
            { rhi::Format::R10G10B10A2_TYPELESS,        DXGI_FORMAT_R10G10B10A2_TYPELESS },
            { rhi::Format::R10G10B10A2_UNORM,           DXGI_FORMAT_R10G10B10A2_UNORM },
            { rhi::Format::R10G10B10A2_UINT,            DXGI_FORMAT_R10G10B10A2_UINT },
            { rhi::Format::R11G11B10_FLOAT,             DXGI_FORMAT_R11G11B10_FLOAT },
            { rhi::Format::R8G8B8A8_TYPELESS,           DXGI_FORMAT_R8G8B8A8_TYPELESS },
            { rhi::Format::R8G8B8A8_UNORM,              DXGI_FORMAT_R8G8B8A8_UNORM },
            { rhi::Format::R8G8B8A8_UNORM_SRGB,         DXGI_FORMAT_R8G8B8A8_UNORM_SRGB },
            { rhi::Format::R8G8B8A8_UINT,               DXGI_FORMAT_R8G8B8A8_UINT },
            { rhi::Format::R8G8B8A8_SNORM,              DXGI_FORMAT_R8G8B8A8_SNORM },
            { rhi::Format::R8G8B8A8_SINT,               DXGI_FORMAT_R8G8B8A8_SINT },
            { rhi::Format::R16G16_TYPELESS,             DXGI_FORMAT_R16G16_TYPELESS },
            { rhi::Format::R16G16_FLOAT,                DXGI_FORMAT_R16G16_FLOAT },
            { rhi::Format::R16G16_UNORM,                DXGI_FORMAT_R16G16_UNORM },
            { rhi::Format::R16G16_UINT,                 DXGI_FORMAT_R16G16_UINT },
            { rhi::Format::R16G16_SNORM,                DXGI_FORMAT_R16G16_SNORM },
            { rhi::Format::R16G16_SINT,                 DXGI_FORMAT_R16G16_SINT },
            { rhi::Format::R32_TYPELESS,                DXGI_FORMAT_R32_TYPELESS },
            { rhi::Format::D32_FLOAT,                   DXGI_FORMAT_D32_FLOAT },
            { rhi::Format::R32_FLOAT,                   DXGI_FORMAT_R32_FLOAT },
            { rhi::Format::R32_UINT,                    DXGI_FORMAT_R32_UINT },
            { rhi::Format::R32_SINT,                    DXGI_FORMAT_R32_SINT },
            { rhi::Format::R24G8_TYPELESS,              DXGI_FORMAT_R24G8_TYPELESS },
            { rhi::Format::D24_UNORM_S8_UINT,           DXGI_FORMAT_D24_UNORM_S8_UINT },
            { rhi::Format::R24_UNORM_X8_TYPELESS,       DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
            { rhi::Format::X24_TYPELESS_G8_UINT,        DXGI_FORMAT_X24_TYPELESS_G8_UINT },
            { rhi::Format::R8G8_TYPELESS,               DXGI_FORMAT_R8G8_TYPELESS },
            { rhi::Format::R8G8_UNORM,                  DXGI_FORMAT_R8G8_UNORM },
            { rhi::Format::R8G8_UINT,                   DXGI_FORMAT_R8G8_UINT },
            { rhi::Format::R8G8_SNORM,                  DXGI_FORMAT_R8G8_SNORM },
            { rhi::Format::R8G8_SINT,                   DXGI_FORMAT_R8G8_SINT },
            { rhi::Format::R16_TYPELESS,                DXGI_FORMAT_R16_TYPELESS },
            { rhi::Format::R16_FLOAT,                   DXGI_FORMAT_R16_FLOAT },
            { rhi::Format::D16_UNORM,                   DXGI_FORMAT_D16_UNORM },
            { rhi::Format::R16_UNORM,                   DXGI_FORMAT_R16_UNORM },
            { rhi::Format::R16_UINT,                    DXGI_FORMAT_R16_UINT },
            { rhi::Format::R16_SNORM,                   DXGI_FORMAT_R16_SNORM },
            { rhi::Format::R16_SINT,                    DXGI_FORMAT_R16_SINT },
            { rhi::Format::R8_TYPELESS,                 DXGI_FORMAT_R8_TYPELESS },
            { rhi::Format::R8_UNORM,                    DXGI_FORMAT_R8_UNORM },
            { rhi::Format::R8_UINT,                     DXGI_FORMAT_R8_UINT },
            { rhi::Format::R8_SNORM,                    DXGI_FORMAT_R8_SNORM },
            { rhi::Format::R8_SINT,                     DXGI_FORMAT_R8_SINT },
            { rhi::Format::A8_UNORM,                    DXGI_FORMAT_A8_UNORM },
            { rhi::Format::R1_UNORM,                    DXGI_FORMAT_R1_UNORM },
            { rhi::Format::R9G9B9E5_SHAREDEXP,          DXGI_FORMAT_R9G9B9E5_SHAREDEXP },
            { rhi::Format::R8G8_B8G8_UNORM,             DXGI_FORMAT_R8G8_B8G8_UNORM },
            { rhi::Format::G8R8_G8B8_UNORM,             DXGI_FORMAT_G8R8_G8B8_UNORM },
            { rhi::Format::BC1_TYPELESS,                DXGI_FORMAT_BC1_TYPELESS },
            { rhi::Format::BC1_UNORM,                   DXGI_FORMAT_BC1_UNORM },
            { rhi::Format::BC1_UNORM_SRGB,              DXGI_FORMAT_BC1_UNORM_SRGB },
            { rhi::Format::BC2_TYPELESS,                DXGI_FORMAT_BC2_TYPELESS },
            { rhi::Format::BC2_UNORM,                   DXGI_FORMAT_BC2_UNORM },
            { rhi::Format::BC2_UNORM_SRGB,              DXGI_FORMAT_BC2_UNORM_SRGB },
            { rhi::Format::BC3_TYPELESS,                DXGI_FORMAT_BC3_TYPELESS },
            { rhi::Format::BC3_UNORM,                   DXGI_FORMAT_BC3_UNORM },
            { rhi::Format::BC3_UNORM_SRGB,              DXGI_FORMAT_BC3_UNORM_SRGB },
            { rhi::Format::BC4_TYPELESS,                DXGI_FORMAT_BC4_TYPELESS },
            { rhi::Format::BC4_UNORM,                   DXGI_FORMAT_BC4_UNORM },
            { rhi::Format::BC4_SNORM,                   DXGI_FORMAT_BC4_SNORM },
            { rhi::Format::BC5_TYPELESS,                DXGI_FORMAT_BC5_TYPELESS },
            { rhi::Format::BC5_UNORM,                   DXGI_FORMAT_BC5_UNORM },
            { rhi::Format::BC5_SNORM,                   DXGI_FORMAT_BC5_SNORM },
            { rhi::Format::B5G6R5_UNORM,                DXGI_FORMAT_B5G6R5_UNORM },
            { rhi::Format::B5G5R5A1_UNORM,              DXGI_FORMAT_B5G5R5A1_UNORM },
            { rhi::Format::B8G8R8A8_UNORM,              DXGI_FORMAT_B8G8R8A8_UNORM },
            { rhi::Format::B8G8R8X8_UNORM,              DXGI_FORMAT_B8G8R8X8_UNORM },
            { rhi::Format::R10G10B10_XR_BIAS_A2_UNORM,  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { rhi::Format::B8G8R8A8_TYPELESS,           DXGI_FORMAT_B8G8R8A8_TYPELESS },
            { rhi::Format::B8G8R8A8_UNORM_SRGB,         DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
            { rhi::Format::B8G8R8X8_TYPELESS,           DXGI_FORMAT_B8G8R8X8_TYPELESS },
            { rhi::Format::B8G8R8X8_UNORM_SRGB,         DXGI_FORMAT_B8G8R8X8_UNORM_SRGB },
            { rhi::Format::BC6H_TYPELESS,               DXGI_FORMAT_BC6H_TYPELESS },
            { rhi::Format::BC6H_UF16,                   DXGI_FORMAT_BC6H_UF16 },
            { rhi::Format::BC6H_SF16,                   DXGI_FORMAT_BC6H_SF16 },
            { rhi::Format::BC7_TYPELESS,                DXGI_FORMAT_BC7_TYPELESS },
            { rhi::Format::BC7_UNORM,                   DXGI_FORMAT_BC7_UNORM },
            { rhi::Format::BC7_UNORM_SRGB,              DXGI_FORMAT_BC7_UNORM_SRGB }
        };

        static std::unordered_map<rhi::ResourceFlags, D3D12_RESOURCE_FLAGS> s_ResourceFlagsToD3D12ResourceFlags
        {
            { rhi::ResourceFlags::None,                             D3D12_RESOURCE_FLAG_NONE },
            { rhi::ResourceFlags::AllowRenderTarget,                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET },
            { rhi::ResourceFlags::AllowDepthStencil,                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL },
            { rhi::ResourceFlags::AllowUnorderedAccess,             D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS },
            { rhi::ResourceFlags::DenyShaderResource,               D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE },
            { rhi::ResourceFlags::AllowSimultaneousAccess,          D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS },
            { rhi::ResourceFlags::RaytracingAccelerationStructure,  D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE }
        };
    } // namespace unnamed

    DXGI_FORMAT GetDXGIFormat(rhi::Format format)
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

    D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(rhi::ResourceFlags flags)
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

    D3D12_RESOURCE_DIMENSION rhi::d3d12::GetD3D12ResourceDimension(rhi::TextureDimension dimension)
    {
        switch (dimension)
        {
        case rhi::TextureDimension::Unknown: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        case rhi::TextureDimension::Texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case rhi::TextureDimension::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case rhi::TextureDimension::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            UNREACHABLE("Unsupported texture dimension for D3D12 resource creation.");
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }

    D3D12_RESOURCE_STATES GetD3D12ResourceState(rhi::ResourceState state)
    {
        D3D12_RESOURCE_STATES d3dState = D3D12_RESOURCE_STATE_COMMON;

        if (HasFlag(state, rhi::ResourceState::Common))                  d3dState |= D3D12_RESOURCE_STATE_COMMON;
        if (HasFlag(state, rhi::ResourceState::VertexAndConstantBuffer)) d3dState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (HasFlag(state, rhi::ResourceState::IndexBuffer))             d3dState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (HasFlag(state, rhi::ResourceState::RenderTarget))            d3dState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (HasFlag(state, rhi::ResourceState::UnorderedAccess))         d3dState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (HasFlag(state, rhi::ResourceState::DepthWrite))              d3dState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (HasFlag(state, rhi::ResourceState::DepthRead))               d3dState |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (HasFlag(state, rhi::ResourceState::AllShaderResource))    d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, rhi::ResourceState::NonPixelShaderResource))  d3dState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, rhi::ResourceState::PixelShaderResource))     d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (HasFlag(state, rhi::ResourceState::CopyDest))                d3dState |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (HasFlag(state, rhi::ResourceState::CopySource))              d3dState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (HasFlag(state, rhi::ResourceState::ResolveDest))             d3dState |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        if (HasFlag(state, rhi::ResourceState::ResolveSource))           d3dState |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        if (HasFlag(state, rhi::ResourceState::IndirectArgument))        d3dState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (HasFlag(state, rhi::ResourceState::Present))                 d3dState |= D3D12_RESOURCE_STATE_PRESENT;

        return d3dState;
    }

    D3D12_BARRIER_SYNC GetD3D12SyncFlags(rhi::ResourceState state)
    {
        D3D12_BARRIER_SYNC flags = D3D12_BARRIER_SYNC_NONE;

        if (HasFlag(state, rhi::ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        //if (HasFlag(state, rhi::ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_SYNC_INDEX_INPUT;
        if (HasFlag(state, rhi::ResourceState::RenderTarget))            flags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
        if (HasFlag(state, rhi::ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_SYNC_ALL_SHADING;
        if (HasAnyFlag(state, rhi::ResourceState::AllDSV))               flags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
        if (HasFlag(state, rhi::ResourceState::NonPixelShaderResource))  flags |= D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
        if (HasFlag(state, rhi::ResourceState::PixelShaderResource))     flags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
        if (HasAnyFlag(state, rhi::ResourceState::AllCopy))              flags |= D3D12_BARRIER_SYNC_COPY;
        if (HasAnyFlag(state, rhi::ResourceState::AllResolve))           flags |= D3D12_BARRIER_SYNC_RESOLVE;
        if (HasFlag(state, rhi::ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
        if (HasFlag(state, rhi::ResourceState::Split))                   flags |= D3D12_BARRIER_SYNC_SPLIT;
        if (flags == D3D12_BARRIER_SYNC_NONE)                       flags = D3D12_BARRIER_SYNC_ALL; // Fallback to ALL if no specific sync is set

        return flags;
    }

    D3D12_BARRIER_ACCESS GetD3D12AccessFlags(rhi::ResourceState state)
    {
        D3D12_BARRIER_ACCESS flags = D3D12_BARRIER_ACCESS_COMMON;

        if (HasFlag(state, rhi::ResourceState::Common))                  flags |= D3D12_BARRIER_ACCESS_COMMON;
        //if (HasFlag(state, rhi::ResourceState::VertexAndConstantBuffer)) flags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER | D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
        //if (HasFlag(state, rhi::ResourceState::IndexBuffer))             flags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
        if (HasFlag(state, rhi::ResourceState::RenderTarget))            flags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
        if (HasFlag(state, rhi::ResourceState::UnorderedAccess))         flags |= D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
        if (HasFlag(state, rhi::ResourceState::DepthWrite))              flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, rhi::ResourceState::DepthRead))               flags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, rhi::ResourceState::AllShaderResource))    flags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
        if (HasFlag(state, rhi::ResourceState::CopyDest))                flags |= D3D12_BARRIER_ACCESS_COPY_DEST;
        if (HasFlag(state, rhi::ResourceState::CopySource))              flags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
        if (HasFlag(state, rhi::ResourceState::IndirectArgument))        flags |= D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
        if (HasFlag(state, rhi::ResourceState::ResolveDest))             flags |= D3D12_BARRIER_ACCESS_RESOLVE_DEST;
        if (HasFlag(state, rhi::ResourceState::ResolveSource))           flags |= D3D12_BARRIER_ACCESS_RESOLVE_SOURCE;

        return flags;
    }

    D3D12_BARRIER_LAYOUT GetD3D12Layout(rhi::ResourceState state)
    {
        // TODO: it would be better to use queue-specific layouts here

        if (HasFlag(state, rhi::ResourceState::RenderTarget))            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        if (HasFlag(state, rhi::ResourceState::UnorderedAccess))         return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
        if (HasFlag(state, rhi::ResourceState::DepthWrite))              return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        if (HasFlag(state, rhi::ResourceState::DepthRead))               return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
        if (HasAnyFlag(state, rhi::ResourceState::AllShaderResource))    return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        if (HasFlag(state, rhi::ResourceState::CopyDest))                return D3D12_BARRIER_LAYOUT_COPY_DEST;
        if (HasFlag(state, rhi::ResourceState::CopySource))              return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        if (HasFlag(state, rhi::ResourceState::ResolveDest))             return D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
        if (HasFlag(state, rhi::ResourceState::ResolveSource))           return D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
        if (HasFlag(state, rhi::ResourceState::Present))                 return D3D12_BARRIER_LAYOUT_PRESENT;

        UNREACHABLE("Unsupported resource state for layout conversion.");
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }

    D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(rhi::CommandListType type)
    {
        switch (type)
        {
        case rhi::CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case rhi::CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case rhi::CommandListType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
            UNREACHABLE("Unsupported command list type for D3D12 command list creation.");
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    D3D12_PREDICATION_OP GetD3D12PredicationOp(rhi::PredicationOperation op)
    {
        switch (op)
        {
        case rhi::PredicationOperation::EqualZero: return D3D12_PREDICATION_OP_EQUAL_ZERO;
        case rhi::PredicationOperation::NotEqualZero: return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;
        default:
            UNREACHABLE("Unsupported predication operation for D3D12 command list predication.");
            return D3D12_PREDICATION_OP_EQUAL_ZERO;
        }
    }

    D3D12_QUERY_TYPE GetD3D12QueryType(rhi::QueryType type)
    {
        switch (type)
        {
        case rhi::QueryType::Occlusion: return D3D12_QUERY_TYPE_OCCLUSION;
        //case rhi::QueryType::BinaryOcclusion: return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
        case rhi::QueryType::Timestamp: return D3D12_QUERY_TYPE_TIMESTAMP;
        case rhi::QueryType::PipelineStatistics: return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
        default:
            UNREACHABLE("Unsupported query type for D3D12 command list query operations.");
            return D3D12_QUERY_TYPE_OCCLUSION;
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(rhi::PrimitiveTopology primitiveTopology)
    {
        switch (primitiveTopology)
        {
        case rhi::PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case rhi::PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case rhi::PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case rhi::PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case rhi::PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case rhi::PrimitiveTopology::PatchList1: return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList2: return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList3: return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList4: return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList5: return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList6: return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList7: return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList8: return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList9: return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList10: return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList11: return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList12: return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList13: return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList14: return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList15: return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList16: return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList17: return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList18: return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList19: return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList20: return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList21: return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList22: return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList23: return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList24: return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList25: return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList26: return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList27: return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList28: return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList29: return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList30: return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList31: return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
        case rhi::PrimitiveTopology::PatchList32: return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
        default:
            UNREACHABLE("Unsupported primitive topology.");
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    D3D12_QUERY_HEAP_TYPE GetD3D12QueryHeapType(rhi::QueryHeapType type)
    {
        switch (type)
        {
        case rhi::QueryHeapType::Occlusion: return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        case rhi::QueryHeapType::Timestamp: return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        case rhi::QueryHeapType::PipelineStatistics: return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        case rhi::QueryHeapType::PipelineStatistics1: return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS1;
        default:
            UNREACHABLE("Unsupported query heap type.");
            return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        }
    }

    D3D12_BLEND GetD3D12Blend(rhi::Blend blend)
    {
        switch (blend)
        {
        case rhi::Blend::Zero: return D3D12_BLEND_ZERO;
        case rhi::Blend::One: return D3D12_BLEND_ONE;
        case rhi::Blend::SrcColor: return D3D12_BLEND_SRC_COLOR;
        case rhi::Blend::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
        case rhi::Blend::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
        case rhi::Blend::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
        case rhi::Blend::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
        case rhi::Blend::InvDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
        case rhi::Blend::DestColor: return D3D12_BLEND_DEST_COLOR;
        case rhi::Blend::InvDestColor: return D3D12_BLEND_INV_DEST_COLOR;
        case rhi::Blend::SrcAlphaSat: return D3D12_BLEND_SRC_ALPHA_SAT;
        case rhi::Blend::BlendFactor: return D3D12_BLEND_BLEND_FACTOR;
        case rhi::Blend::InvBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
        case rhi::Blend::Src1Color: return D3D12_BLEND_SRC1_COLOR;
        case rhi::Blend::InvSrc1Color: return D3D12_BLEND_INV_SRC1_COLOR;
        case rhi::Blend::Src1Alpha: return D3D12_BLEND_SRC1_ALPHA;
        case rhi::Blend::InvSrc1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
        case rhi::Blend::AlphaFactor: return D3D12_BLEND_ALPHA_FACTOR;
        case rhi::Blend::InvAlphaFactor: return D3D12_BLEND_INV_ALPHA_FACTOR;
        default:
            UNREACHABLE("Unsupported blend type.");
            return D3D12_BLEND_ZERO;
        }
    }

    D3D12_BLEND_OP GetD3D12BlendOp(rhi::BlendOpType blendOp)
    {
        switch (blendOp)
        {
        case rhi::BlendOpType::Add: return D3D12_BLEND_OP_ADD;
        case rhi::BlendOpType::Subtract: return D3D12_BLEND_OP_SUBTRACT;
        case rhi::BlendOpType::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
        case rhi::BlendOpType::Min: return D3D12_BLEND_OP_MIN;
        case rhi::BlendOpType::Max: return D3D12_BLEND_OP_MAX;
        default:
            UNREACHABLE("Unsupported blend operation.");
            return D3D12_BLEND_OP_ADD;
        }
    }

    D3D12_LOGIC_OP GetD3D12LogicOp(rhi::LogicOp logicOp)
    {
        switch (logicOp)
        {
        case rhi::LogicOp::Clear: return D3D12_LOGIC_OP_CLEAR;
        case rhi::LogicOp::Set: return D3D12_LOGIC_OP_SET;
        case rhi::LogicOp::Copy: return D3D12_LOGIC_OP_COPY;
        case rhi::LogicOp::CopyInverted: return D3D12_LOGIC_OP_COPY_INVERTED;
        case rhi::LogicOp::NoOp: return D3D12_LOGIC_OP_NOOP;
        case rhi::LogicOp::Invert: return D3D12_LOGIC_OP_INVERT;
        case rhi::LogicOp::And: return D3D12_LOGIC_OP_AND;
        case rhi::LogicOp::Nand: return D3D12_LOGIC_OP_NAND;
        case rhi::LogicOp::Or: return D3D12_LOGIC_OP_OR;
        case rhi::LogicOp::Nor: return D3D12_LOGIC_OP_NOR;
        case rhi::LogicOp::Xor: return D3D12_LOGIC_OP_XOR;
        case rhi::LogicOp::Equiv: return D3D12_LOGIC_OP_EQUIV;
        case rhi::LogicOp::AndReverse: return D3D12_LOGIC_OP_AND_REVERSE;
        case rhi::LogicOp::AndInverted: return D3D12_LOGIC_OP_AND_INVERTED;
        case rhi::LogicOp::OrReverse: return D3D12_LOGIC_OP_OR_REVERSE;
        case rhi::LogicOp::OrInverted: return D3D12_LOGIC_OP_OR_INVERTED;
        default:
            UNREACHABLE("Unsupported logic operation.");
            return D3D12_LOGIC_OP_CLEAR;
        }
    }

    D3D12_COLOR_WRITE_ENABLE GetD3D12RenderTargetWriteMask(rhi::ColorWriteEnable colorWriteEnable)
    {
        switch (colorWriteEnable)
        {
        case rhi::ColorWriteEnable::DisableAll: return (D3D12_COLOR_WRITE_ENABLE)0;
        case rhi::ColorWriteEnable::Red: return D3D12_COLOR_WRITE_ENABLE_RED;
        case rhi::ColorWriteEnable::Green: return D3D12_COLOR_WRITE_ENABLE_GREEN;
        case rhi::ColorWriteEnable::Blue: return D3D12_COLOR_WRITE_ENABLE_BLUE;
        case rhi::ColorWriteEnable::Alpha: return D3D12_COLOR_WRITE_ENABLE_ALPHA;
        case rhi::ColorWriteEnable::All: return D3D12_COLOR_WRITE_ENABLE_ALL;
        default:
            UNREACHABLE("Unsupported color write mask");
            return (D3D12_COLOR_WRITE_ENABLE)0;
        }
    }

    D3D12_BLEND_DESC GetD3D12BlendDesc(const rhi::BlendState& blendState)
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

    D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RTBlendState(const rhi::RTBlendState& blendState)
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

    D3D12_FILL_MODE GetD3D12FillMode(rhi::FillMode fillMode)
    {
        switch (fillMode)
        {
        case rhi::FillMode::Solid: return D3D12_FILL_MODE_SOLID;
        case rhi::FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
        default:
            UNREACHABLE("Unsupported fill mode.");
            return D3D12_FILL_MODE_SOLID;
        }
    }

    D3D12_CULL_MODE GetD3D12CullMode(rhi::CullMode cullMode)
    {
        switch (cullMode)
        {
        case rhi::CullMode::None: return D3D12_CULL_MODE_NONE;
        case rhi::CullMode::Back: return D3D12_CULL_MODE_BACK;
        case rhi::CullMode::Front: return D3D12_CULL_MODE_FRONT;
        default:
            UNREACHABLE("Unsupported cull mode.");
            return D3D12_CULL_MODE_NONE;
        }
    }

    D3D12_RASTERIZER_DESC GetD3D12RasterizerDesc(const rhi::RasterizerState& rasterizerState)
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

    D3D12_DEPTH_WRITE_MASK GetD3D12DepthWriteMask(rhi::DepthWriteMask mask)
    {
        switch (mask)
        {
        case rhi::DepthWriteMask::Zero: return D3D12_DEPTH_WRITE_MASK_ZERO;
        case rhi::DepthWriteMask::All: return D3D12_DEPTH_WRITE_MASK_ALL;
        default:
            UNREACHABLE("Unsupported depth write mask.");
            return D3D12_DEPTH_WRITE_MASK_ZERO;
        }
    }

    D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(rhi::ComparisonFunc comparisonFunc)
    {
        switch (comparisonFunc)
        {
        case rhi::ComparisonFunc::None: return D3D12_COMPARISON_FUNC_NONE;
        case rhi::ComparisonFunc::Never: return D3D12_COMPARISON_FUNC_NEVER;
        case rhi::ComparisonFunc::Less: return D3D12_COMPARISON_FUNC_LESS;
        case rhi::ComparisonFunc::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
        case rhi::ComparisonFunc::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case rhi::ComparisonFunc::Greater: return D3D12_COMPARISON_FUNC_GREATER;
        case rhi::ComparisonFunc::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case rhi::ComparisonFunc::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case rhi::ComparisonFunc::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            UNREACHABLE("Unsupported comparison function.");
            return D3D12_COMPARISON_FUNC_NONE;
        }
    }

    D3D12_STENCIL_OP GetD3D12StencilOp(rhi::StencilOp stencilOp)
    {
        switch (stencilOp)
        {
            case rhi::StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
            case rhi::StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
            case rhi::StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
            case rhi::StencilOp::IncrSat: return D3D12_STENCIL_OP_INCR_SAT;
            case rhi::StencilOp::DecrSat: return D3D12_STENCIL_OP_DECR_SAT;
            case rhi::StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
            case rhi::StencilOp::Incr: return D3D12_STENCIL_OP_INCR;
            case rhi::StencilOp::Decr: return D3D12_STENCIL_OP_DECR;
            default:
                UNREACHABLE("Unsupported stencil op.");
                return D3D12_STENCIL_OP_ZERO;
        }
    }

    D3D12_DEPTH_STENCILOP_DESC GetD3D12StencilOpDesc(const rhi::DepthStencilOpDesc& depthStencilOpDesc)
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

    D3D12_DEPTH_STENCIL_DESC GetD3D12DepthStencilDesc(const rhi::DepthStencilState depthStencilState)
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

    D3D12_CLEAR_FLAGS GetD3D12ClearFlags(rhi::ClearFlags clearFlags)
    {
        switch (clearFlags)
        {
        case rhi::ClearFlags::DepthStencil: return D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
        case rhi::ClearFlags::Depth: return D3D12_CLEAR_FLAG_DEPTH;
        case rhi::ClearFlags::Stencil: return D3D12_CLEAR_FLAG_STENCIL;
        default:
            UNREACHABLE("Unsupported clear flag.");
            return D3D12_CLEAR_FLAG_DEPTH;
        }
    }

    D3D12_HEAP_TYPE GetD3D12HeapType(rhi::HeapType type)
    {
        switch (type)
        {
        case rhi::HeapType::Default: return D3D12_HEAP_TYPE_DEFAULT;
        case rhi::HeapType::Upload: return D3D12_HEAP_TYPE_UPLOAD;
        case rhi::HeapType::GPUUpload: return D3D12_HEAP_TYPE_GPU_UPLOAD;
        case rhi::HeapType::Readback: return D3D12_HEAP_TYPE_READBACK;
        case rhi::HeapType::Custom: return D3D12_HEAP_TYPE_CUSTOM;
        default:
            UNREACHABLE("Unsupported heap type.");
            return D3D12_HEAP_TYPE_DEFAULT;
        }
    }

    D3D12_CPU_PAGE_PROPERTY GetD3D12CPUPageProperty(rhi::CPUPageProperty property)
    {
        switch (property)
        {
        case rhi::CPUPageProperty::Unknown: return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        case rhi::CPUPageProperty::NotAvailable: return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        case rhi::CPUPageProperty::WriteCombine: return D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
        case rhi::CPUPageProperty::Writeback: return D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
        default:
            UNREACHABLE("Unsupported CPU page property.");
            return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        }
    }

    D3D12_MEMORY_POOL GetD3D12MemoryPool(rhi::MemoryPool memoryPool)
    {
        switch (memoryPool)
        {
        case rhi::MemoryPool::Unknown: return D3D12_MEMORY_POOL_UNKNOWN;
        case rhi::MemoryPool::L0: return D3D12_MEMORY_POOL_L0;
        case rhi::MemoryPool::L1: return D3D12_MEMORY_POOL_L1;
        default:
            UNREACHABLE("Unsupported memory pool.");
            return D3D12_MEMORY_POOL_UNKNOWN;
        }
    }

    D3D12_HEAP_PROPERTIES GetD3D12HeapProperties(rhi::HeapProperties properties)
    {
        D3D12_HEAP_PROPERTIES result =
        {
            .Type = GetD3D12HeapType(properties.Type),
            .CPUPageProperty = GetD3D12CPUPageProperty(properties.CPUPageProperty),
            .MemoryPoolPreference = GetD3D12MemoryPool(properties.MemoryPoolPreference),
            .CreationNodeMask = properties.CreationNodeMask,
            .VisibleNodeMask = properties.VisibleNodeMask
        };

        return result;
    }

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const rhi::BufferDescription& description)
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

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const rhi::TextureDescription& description)
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
    D3D12_INDIRECT_ARGUMENT_DESC GetD3D12IndirectArgumentDesc(const rhi::IndirectArgumentDescription argumentDesc)
    {
        D3D12_INDIRECT_ARGUMENT_DESC argument = {};

        switch (argumentDesc.Type)
        {
        case rhi::IndirectArgumentType::Draw:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
            break;
        case rhi::IndirectArgumentType::DrawIndexed:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
            break;
        case rhi::IndirectArgumentType::Dispatch:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
            break;
        case rhi::IndirectArgumentType::VertexBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_VERTEX_BUFFER_VIEW;
            argument.VertexBuffer.Slot = argumentDesc.VertexBuffer.Slot;
            break;
        case rhi::IndirectArgumentType::IndexBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_INDEX_BUFFER_VIEW;
            break;
        case rhi::IndirectArgumentType::Constant:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
            argument.Constant =
            {
                .RootParameterIndex = argumentDesc.Constant.RootParameterIndex,
                .DestOffsetIn32BitValues = argumentDesc.Constant.DestOffsetIn32BitValues,
                .Num32BitValuesToSet = argumentDesc.Constant.Num32BitValuesToSet
            };
            break;
        case rhi::IndirectArgumentType::ConstantBufferView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
            argument.ConstantBufferView.RootParameterIndex = argumentDesc.ConstantBufferView.RootParameterIndex;
            break;
        case rhi::IndirectArgumentType::ShaderResourceView:
            argument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
            argument.ShaderResourceView.RootParameterIndex = argumentDesc.ShaderResourceView.RootParameterIndex;
            break;
        case rhi::IndirectArgumentType::UnorderedResourceView:
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
