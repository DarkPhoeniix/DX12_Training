#pragma once

#include "Fence.h"

namespace rhi::vulkan
{
    class VulkanFence final : public rhi::Fence
    {
    public:
        VulkanFence(const VulkanFence&) = delete;
        VulkanFence(VulkanFence&& other) noexcept;
        ~VulkanFence() override;

        VulkanFence& operator=(const VulkanFence&) = delete;
        VulkanFence& operator=(VulkanFence&& other) noexcept;

        void Wait() override;

        void SetValue(std::uint64_t value) override { _fenceValue = value; }
        std::uint64_t GetValue() const override { return _fenceValue; }

        void SetFree(bool isFree) override { _isFree = isFree; }
        bool IsFree() const override { return _isFree; }

        void SetCompletionCallback(const std::function<void()>& callback) override { _cpuCallback = callback; }

        void* GetNative() const override;

    private:
        friend class VulkanDevice;

        VulkanFence(rhi::Device* device, std::uint64_t initialValue);

        vk::Semaphore _semaphore;

        std::uint64_t _fenceValue;
        bool _isFree;

        std::function<void()> _cpuCallback;

        rhi::Device* _device;
    };
} // namespace rhi::vulkan
