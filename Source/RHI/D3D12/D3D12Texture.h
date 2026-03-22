#pragma once

#include "Texture.h"
#include "D3D12Resource.h"

namespace rhi::d3d12
{
    class D3D12Texture final : public rhi::Texture
    {
    public:
        D3D12Texture(const D3D12Texture& other) = delete;
        D3D12Texture(D3D12Texture&& other);
        ~D3D12Texture() override = default;

        D3D12Texture& operator=(const D3D12Texture& other) = delete;
        D3D12Texture& operator=(D3D12Texture&& other) noexcept;

        void* Map(std::uint32_t begin, std::uint32_t end) override;
        void Unmap() override;

        std::uint64_t GetVirtualAddress() override;

        ResourceState GetInitialState() const override;
        ResourceState GetCurrentState() const override;

        const AllocationInfo& GetAllocationInfo() const override;

        void* GetNative() const override;

    private:
        friend class D3D12Device;

        D3D12Texture(rhi::Device* device, const TextureDescription& description, const void* initialData = nullptr, const std::string& name = "");
        D3D12Texture(rhi::Device* device, ID3D12Resource* nativeTexturePtr, const std::string& name = "");

        D3D12Resource _resource;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif
    };
}