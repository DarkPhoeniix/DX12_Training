#pragma once

#include "Buffer.h"
#include "VulkanHelpers.h"

namespace rhi
{
    class Device;
} // namespace rhi

namespace rhi::vulkan
{
    class VulkanBuffer final : public rhi::Buffer
    {
    public:
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&& other) noexcept;
        ~VulkanBuffer() override;

        VulkanBuffer& operator=(const VulkanBuffer&) = delete;
        VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

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
        friend class VulkanDevice;

        VulkanBuffer(rhi::Device* device, VmaAllocator allocator, const rhi::BufferDescription& description, ResourceState initialState = ResourceState::Common, const std::string& name = "");
        VulkanBuffer(rhi::Device* device, vk::Buffer nativeBuffer, const std::string& name = "");

        vk::Buffer    _buffer;
        VmaAllocator  _allocator;   // non-owning
        VmaAllocation _allocation;

        BufferDescription _description;

        ResourceID    _ID;
        ResourceState _initialState;
        ResourceState _currentState;

        std::uint32_t _uavCounterOffset;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
