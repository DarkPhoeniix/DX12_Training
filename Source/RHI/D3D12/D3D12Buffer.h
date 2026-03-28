#pragma once

#include "Buffer.h"
#include "D3D12Resource.h"

#include <string>

namespace rhi
{
    class Device;
}

namespace rhi::d3d12
{
    class D3D12Buffer final : public rhi::Buffer
    {
    public:
        D3D12Buffer(const D3D12Buffer& other) = delete;
        D3D12Buffer(D3D12Buffer&& other) noexcept;
        ~D3D12Buffer() override;

        D3D12Buffer& operator=(const D3D12Buffer& other) = delete;
        D3D12Buffer& operator=(D3D12Buffer&& other) noexcept;

        void* Map(std::uint32_t begin, std::uint32_t end) override;
        void Unmap() override;

        std::uint64_t GetVirtualAddress() override;

        ResourceState GetInitialState() const override;
        ResourceState GetCurrentState() const override;
        void SetCurrentState(ResourceState state) override;

        std::uint32_t GetUAVCounterOffset() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Buffer(rhi::Device* device, const rhi::BufferDescription& description, ResourceState initialState = ResourceState::Common, [[maybe_unused]] const std::string& name = "");
        D3D12Buffer(rhi::Device* device, const rhi::BufferDescription& description, rhi::Heap* heap, std::uint64_t offset, ResourceState initialState = ResourceState::Common, [[maybe_unused]] const std::string& name = "");
        D3D12Buffer(rhi::Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name = "");

        D3D12Resource _resource;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
}
