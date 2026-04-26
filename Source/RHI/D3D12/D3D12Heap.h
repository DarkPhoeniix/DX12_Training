#pragma once

#include "Heap.h"

namespace rhi::d3d12
{
    class D3D12Heap final : public rhi::Heap
    {
    public:
        D3D12Heap(const D3D12Heap& other) = delete;
        D3D12Heap(D3D12Heap&& other) noexcept;
        ~D3D12Heap();

        D3D12Heap& operator=(const D3D12Heap& other) = delete;
        D3D12Heap& operator=(D3D12Heap&& other) noexcept;

        std::shared_ptr<rhi::Buffer> PlaceResource(const rhi::BufferDescription& bufferDesc, rhi::ResourceState state = rhi::ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) override;
        std::shared_ptr<rhi::Texture> PlaceResource(const rhi::TextureDescription& textureDesc, rhi::ResourceState state = rhi::ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) override;

        void Reset();

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Heap(rhi::Device* device, const rhi::HeapDescription& description, const std::string& name = "");

        ComPtr<ID3D12Heap> _heap;
        rhi::HeapDescription _description;

        std::uint64_t _resourceOffset;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
