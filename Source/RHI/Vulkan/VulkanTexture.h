#pragma once

#include "Texture.h"
#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    class VulkanTexture final : public rhi::Texture
    {
    public:
        VulkanTexture(const VulkanTexture&) = delete;
        VulkanTexture(VulkanTexture&& other) noexcept;
        ~VulkanTexture() override;

        VulkanTexture& operator=(const VulkanTexture&) = delete;
        VulkanTexture& operator=(VulkanTexture&& other) noexcept;

        void* Map(std::uint32_t begin, std::uint32_t end) override;
        void Unmap() override;

        std::uint64_t GetVirtualAddress() override;

        ResourceState GetInitialState() const override;
        ResourceState GetCurrentState() const override;
        void SetCurrentState(ResourceState state) override;

        const TextureDescription& GetDescription() const override;

        std::uint32_t GetWidth() const override;
        std::uint32_t GetHeight() const override;
        std::uint32_t GetMipLevels() const override;
        std::uint32_t GetDepthOrArraySize() override;
        Format GetFormat() const override;
        TextureDimension GetDimension() const override;

        const ResourceID& GetID() const override;

        void* GetNative() const override;

    private:
        friend class VulkanDevice;
        friend class VulkanSwapChain;

        VulkanTexture(rhi::Device* device, VmaAllocator allocator, const TextureDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        VulkanTexture(rhi::Device* device, vk::Image nativeImage, const std::string& name = "");

        vk::Image     _image;
        VmaAllocator  _allocator;   // non-owning, nullptr for wrapped native images
        VmaAllocation _allocation;  // nullptr for wrapped native images

        TextureDescription _description;

        ResourceID    _ID;
        ResourceState _initialState;
        ResourceState _currentState;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
