#pragma once

#include "CommandQueue.h"

#include "CommandList.h"
#include "Device.h"

namespace rhi::vulkan
{
    class VulkanDevice;

    class VulkanCommandQueue final : public rhi::CommandQueue
    {
    public:
        VulkanCommandQueue(const VulkanCommandQueue& other) = delete;
        VulkanCommandQueue(VulkanCommandQueue&& other) noexcept;
        ~VulkanCommandQueue() override;

        VulkanCommandQueue& operator=(const VulkanCommandQueue& other) = delete;
        VulkanCommandQueue& operator=(VulkanCommandQueue&& other) noexcept;

        void ExecuteCommandLists(const std::vector<rhi::CommandList*>& commandLists) override;

        void Signal(rhi::Fence* fence, std::uint64_t value) override;
        void Wait(rhi::Fence* fence, std::uint64_t value) override;

        std::uint64_t GetTimestampFrequency() const override;

        rhi::CommandListType GetType() const override;

        // The Vulkan queue family this queue belongs to (present support, command pools).
        std::uint32_t GetQueueFamilyIndex() const;

        void* GetNative() const override;

    private:
        friend class VulkanDevice;

        VulkanCommandQueue(rhi::Device* device, rhi::CommandListType type, std::uint32_t queueFamilyIndex);

        vk::Queue _queue;
        rhi::CommandListType _type;
        std::uint32_t _queueFamilyIndex;

        rhi::Device* _device;
    };
} // namespace rhi::vulkan
