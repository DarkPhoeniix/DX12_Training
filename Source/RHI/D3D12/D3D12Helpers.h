#pragma once

#include "Format.h"
#include "ResourceCommon.h"
#include "QueryHeap.h"
#include "CommandList.h"

namespace rhi
{
    struct IndirectArgumentDescription;
}

namespace rhi::d3d12
{
    template<typename T>
    concept D3D12Type = std::derived_from<T, IUnknown>;
    template<typename T>
    concept D3D12Object = std::derived_from<T, ID3D12Object>;

    template<D3D12Type T>
    inline T* D3D12Cast(void* ptr)
    {
        ASSERT(ptr != nullptr, "Trying to cast a null pointer.");
        return reinterpret_cast<T*>(ptr);
    }

    template<D3D12Object T>
    inline void SetD3D12Name(T* object, const std::string& name)
    {
#if ENABLE_DEBUG_NAMES
        if (object && !name.empty())
        {
            object->SetName(std::wstring(name.begin(), name.end()).c_str());
        }
#endif // ENABLE_DEBUG_NAMES
    }

    DXGI_FORMAT GetDXGIFormat(Format format);
    D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags(ResourceFlags flags);
    D3D12_RESOURCE_DIMENSION GetD3D12ResourceDimension(TextureDimension dimension);

    D3D12_RESOURCE_STATES GetD3D12ResourceState(ResourceState state);
    D3D12_BARRIER_SYNC GetD3D12SyncFlags(ResourceState state);
    D3D12_BARRIER_ACCESS GetD3D12AccessFlags(ResourceState state);
    D3D12_BARRIER_LAYOUT GetD3D12Layout(ResourceState state);

    D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(CommandListType type);
    D3D12_PREDICATION_OP GetD3D12PredicationOp(PredicationOperation op);
    D3D12_QUERY_TYPE GetD3D12QueryType(QueryType type);
    D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(PrimitiveTopology primitiveTopology);

    D3D12_QUERY_HEAP_TYPE GetD3D12QueryHeapType(QueryHeapType type);

    D3D12_BLEND GetD3D12Blend(Blend blend);
    D3D12_BLEND_OP GetD3D12BlendOp(BlendOpType blendOp);
    D3D12_LOGIC_OP GetD3D12LogicOp(LogicOp logicOp);
    D3D12_COLOR_WRITE_ENABLE GetD3D12RenderTargetWriteMask(ColorWriteEnable colorWriteEnable);
    D3D12_BLEND_DESC GetD3D12BlendDesc(const BlendState& blendState);
    D3D12_RENDER_TARGET_BLEND_DESC GetD3D12RTBlendState(const RTBlendState& blendState);

    D3D12_FILL_MODE GetD3D12FillMode(FillMode fillMode);
    D3D12_CULL_MODE GetD3D12CullMode(CullMode cullMode);
    D3D12_RASTERIZER_DESC GetD3D12RasterizerDesc(const RasterizerState& rasterizerState);

    D3D12_DEPTH_WRITE_MASK GetD3D12DepthWriteMask(DepthWriteMask mask);
    D3D12_COMPARISON_FUNC GetD3D12ComparisonFunc(ComparisonFunc comparisonFunc);
    D3D12_STENCIL_OP GetD3D12StencilOp(StencilOp stencilOp);
    D3D12_DEPTH_STENCILOP_DESC GetD3D12StencilOpDesc(const DepthStencilOpDesc& depthStencilOpDesc);
    D3D12_DEPTH_STENCIL_DESC GetD3D12DepthStencilDesc(const DepthStencilState depthStencilState);

    D3D12_CLEAR_FLAGS GetD3D12ClearFlags(ClearFlags clearFlags);

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const BufferDescription& description);
    D3D12_RESOURCE_DESC GetD3D12ResourceDesc(const TextureDescription& description);

    D3D12_INDIRECT_ARGUMENT_DESC GetD3D12IndirectArgumentDesc(const IndirectArgumentDescription argumentDesc);
} // namespace rhi::d3d12
