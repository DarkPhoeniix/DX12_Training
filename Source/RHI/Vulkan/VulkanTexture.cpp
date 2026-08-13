
#include "RHI_PCH.h"

#include "VulkanTexture.h"

#include "ResourceIdGenerator.h"

#include <vma/vk_mem_alloc.h>

namespace rhi::vulkan
{
    VulkanTexture::VulkanTexture(rhi::Device* device, VmaAllocator allocator, const TextureDescription& description, ResourceState initialState, const std::string& name)
        : _image(nullptr)
        , _allocator(allocator)
        , _allocation(nullptr)
        , _description(description)
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(initialState)
        , _currentState(initialState)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        const vk::ImageCreateInfo createInfo =
        {
            .imageType = GetVkImageType(description.Dimension),
            .format = GetVkFormat(description.Format),
            .extent =
            {
                .width = description.Width,
                .height = description.Height,
                .depth = description.Dimension == TextureDimension::Texture3D ? description.DepthOrArraySize : 1u
            },
            .mipLevels = description.MipLevels,
            .arrayLayers = description.Dimension == TextureDimension::Texture3D ? 1u : description.DepthOrArraySize,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal,
            .usage = GetVkImageUsage(description.Flags),
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };

        VmaAllocationCreateInfo allocationInfo = {};
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;

        VkImage image = VK_NULL_HANDLE;
        VkResult result = vmaCreateImage(_allocator,
            reinterpret_cast<const VkImageCreateInfo*>(&createInfo),
            &allocationInfo,
            &image,
            &_allocation,
            nullptr);
        VK_CHECK(static_cast<vk::Result>(result), "Failed to allocate texture");

        _image = image;

        SetVulkanName(VulkanCast<vk::Device>(_device->GetNative()), _image, name);
    }

    VulkanTexture::VulkanTexture(rhi::Device* device, vk::Image nativeImage, const std::string& name)
        : _image(nativeImage)
        , _allocator(nullptr)
        , _allocation(nullptr)
        , _description{}
        , _ID(rhi::ResourceIdGenerator::GenerateID())
        , _initialState(ResourceState::Common)
        , _currentState(ResourceState::Common)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        SetVulkanName(VulkanCast<vk::Device>(_device->GetNative()), _image, name);
    }

    VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept
        : rhi::Texture(std::move(other))
        , _image(std::exchange(other._image, nullptr))
        , _allocator(std::exchange(other._allocator, nullptr))
        , _allocation(std::exchange(other._allocation, nullptr))
        , _description(std::move(other._description))
        , _ID(std::move(other._ID))
        , _initialState(other._initialState)
        , _currentState(other._currentState)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanTexture::~VulkanTexture()
    {
        if (_allocation)
        {
            vmaDestroyImage(_allocator, _image, _allocation);
        }
    }

    VulkanTexture& VulkanTexture::operator=(VulkanTexture&& other) noexcept
    {
        if (this != &other)
        {
            rhi::Texture::operator=(std::move(other));
            _image       = std::exchange(other._image, nullptr);
            _allocator   = std::exchange(other._allocator, nullptr);
            _allocation  = std::exchange(other._allocation, nullptr);
            _description = std::move(other._description);
            _ID          = std::move(other._ID);
            _initialState = other._initialState;
            _currentState = other._currentState;
            _device      = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    void* VulkanTexture::Map(std::uint32_t, std::uint32_t)
    {
        ASSERT(_allocation, "Trying to map a texture that owns no allocation.");

        void* data = nullptr;
        VkResult result = vmaMapMemory(_allocator, _allocation, &data);
        VK_CHECK(static_cast<vk::Result>(result), "Failed to map texture");

        return data;
    }

    void VulkanTexture::Unmap()
    {
        ASSERT(_allocation, "Trying to unmap a texture that owns no allocation.");
        vmaUnmapMemory(_allocator, _allocation);
    }

    std::uint64_t VulkanTexture::GetVirtualAddress()
    {
        // Vulkan does not expose GPU virtual addresses for images
        return 0;
    }

    ResourceState VulkanTexture::GetInitialState() const
    {
        return _initialState;
    }

    ResourceState VulkanTexture::GetCurrentState() const
    {
        return _currentState;
    }

    void VulkanTexture::SetCurrentState(ResourceState state)
    {
        _currentState = state;
    }

    const TextureDescription& VulkanTexture::GetDescription() const
    {
        return _description;
    }

    std::uint32_t VulkanTexture::GetWidth() const
    {
        return _description.Width;
    }

    std::uint32_t VulkanTexture::GetHeight() const
    {
        return _description.Height;
    }

    std::uint32_t VulkanTexture::GetMipLevels() const
    {
        return _description.MipLevels;
    }

    std::uint32_t VulkanTexture::GetDepthOrArraySize()
    {
        return _description.DepthOrArraySize;
    }

    Format VulkanTexture::GetFormat() const
    {
        return _description.Format;
    }

    TextureDimension VulkanTexture::GetDimension() const
    {
        return _description.Dimension;
    }

    const ResourceID& VulkanTexture::GetID() const
    {
        return _ID;
    }

    void* VulkanTexture::GetNative() const
    {
        return VulkanNative(_image);
    }
} // namespace rhi::vulkan
