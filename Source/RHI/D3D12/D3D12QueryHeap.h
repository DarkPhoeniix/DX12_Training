#pragma once

#include "QueryHeap.h"

namespace rhi
{
    class Device;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12QueryHeap final : public rhi::QueryHeap
    {
    public:
        D3D12QueryHeap(const D3D12QueryHeap& other) = delete;
        D3D12QueryHeap(D3D12QueryHeap&& other) noexcept;
        ~D3D12QueryHeap() override;

        D3D12QueryHeap& operator=(const D3D12QueryHeap& other) = delete;
        D3D12QueryHeap& operator=(D3D12QueryHeap&& other) noexcept;

        QueryHeapType GetType() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12QueryHeap(rhi::Device* device, const QueryHeapDescription& description, const std::string& name = "");

        rhi::QueryHeapType _type;
        ComPtr<ID3D12QueryHeap> _queryHeap;
#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
