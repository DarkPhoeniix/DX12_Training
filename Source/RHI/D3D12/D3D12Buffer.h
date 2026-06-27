#pragma once

#include "Buffer.h"
#include "D3D12Resource.h"

namespace rhi
{
    class Device;
} // namespace rhi

namespace rhi::d3d12
{
    class D3D12Buffer final : public Buffer
    {
    public:
        D3D12Buffer(const D3D12Buffer& other) = delete;
        D3D12Buffer(D3D12Buffer&& other) noexcept;
        ~D3D12Buffer() override;

        D3D12Buffer& operator=(const D3D12Buffer& other) = delete;
        D3D12Buffer& operator=(D3D12Buffer&& other) noexcept;

        void* Map(std::uint32_t begin, std::uint32_t end) override;
        void Unmap() override;

        std::uint64_t GetVirtualAddress(std::uint64_t offset) override;

        ResourceState GetInitialState() const override;
        ResourceState GetCurrentState() const override;
        void SetCurrentState(ResourceState state) override;

        const BufferDescription& GetDescription() const override;
        std::uint32_t GetSize() const override;
        std::uint32_t GetStride() const override;
        std::uint32_t GetElementCount() const override;

        std::uint32_t GetUAVCounterOffset() const override;

        const ResourceID& GetID() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Buffer(Device* device, D3D12MA::Allocator* allocator, const BufferDescription& description, ResourceState initialState = ResourceState::Common, [[maybe_unused]] const std::string& name = "");
        D3D12Buffer(Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name = "");

        BufferDescription _description;

        D3D12Resource _resource;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::d3d12
