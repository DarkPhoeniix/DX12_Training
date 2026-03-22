#pragma once

#include "Heap.h"

namespace rhi::d3d12
{
    class D3D12Resource;
    
    class D3D12Heap final : public rhi::Heap
    {
    public:
        D3D12Heap(const D3D12Heap& other) = delete;
        D3D12Heap(D3D12Heap&& other) noexcept;
        ~D3D12Heap();

        D3D12Heap& operator=(const D3D12Heap& other) = delete;
        D3D12Heap& operator=(D3D12Heap&& other) noexcept;

        void PlaceResource(Buffer& buffer, ResourceState state, std::uint64_t offset) override;
        void PlaceResource(Texture& texture, ResourceState state, std::uint64_t offset) override;

        // Reset the heap, releasing resources.
        void Reset();

        // Inherited via Heap
        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Heap(rhi::Device* device, const rhi::HeapDescription& description);

        ComPtr<ID3D12Heap> _heap;
        rhi::HeapDescription _description;

        std::uint64_t _resourceOffset;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
