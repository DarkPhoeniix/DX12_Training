
#include "RHI_PCH.h"

#include "D3D12QueryHeap.h"

#include "D3D12Helpers.h"

namespace rhi::d3d12
{
    D3D12QueryHeap::D3D12QueryHeap(rhi::Device* device, const QueryHeapDescription& description, const std::string& name)
        : _type(description.Type)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        ID3D12Device* d3d12Device = D3D12Cast<ID3D12Device>(device->GetNative());

        D3D12_QUERY_HEAP_DESC desc =
        {
            .Type = GetD3D12QueryHeapType(description.Type),
            .Count = description.Count,
            .NodeMask = description.NodeMask
        };

        d3d12Device->CreateQueryHeap(&desc, IID_PPV_ARGS(&_queryHeap));

#if ENABLE_DEBUG_NAMES
        SetD3D12Name(_queryHeap.Get(), name);
#endif // ENABLE_DEBUG_NAMES
    }

    D3D12QueryHeap::D3D12QueryHeap(D3D12QueryHeap&& other) noexcept
        : rhi::QueryHeap(std::move(other))
        , _type(other._type)
        , _queryHeap(std::move(other._queryHeap))
#if ENABLE_DEBUG_NAMES
        , _name(other._name)
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12QueryHeap::~D3D12QueryHeap()
    {
    }

    D3D12QueryHeap& D3D12QueryHeap::operator=(D3D12QueryHeap&& other) noexcept
    {
        if (this != &other)
        {
            rhi::QueryHeap::operator=(std::move(other));
            _queryHeap = std::move(other._queryHeap);
            _type = other._type;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    QueryHeapType D3D12QueryHeap::GetType() const
    {
        return _type;
    }

    void* D3D12QueryHeap::GetNative() const
    {
        return static_cast<void*>(_queryHeap.Get());
    }
} // namespace rhi::d3d12
