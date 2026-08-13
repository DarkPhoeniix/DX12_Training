
#include "RHI_PCH.h"

#include "VulkanFence.h"

#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    VulkanFence::VulkanFence(rhi::Device* device, std::uint64_t initialValue)
        : _semaphore(nullptr)
        , _fenceValue(initialValue)
        , _isFree(true)
        , _device(device)
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(device->GetNative());

        const vk::SemaphoreTypeCreateInfo typeInfo =
        {
            .semaphoreType = vk::SemaphoreType::eTimeline,
            .initialValue  = initialValue
        };
        const vk::SemaphoreCreateInfo createInfo =
        {
            .pNext = &typeInfo
        };

        auto [result, semaphore] = vkDevice.createSemaphore(createInfo);
        VK_CHECK(result, "Failed to create timeline semaphore");
        _semaphore = semaphore;
    }

    VulkanFence::VulkanFence(VulkanFence&& other) noexcept
        : _semaphore(std::exchange(other._semaphore, nullptr))
        , _fenceValue(other._fenceValue)
        , _isFree(other._isFree)
        , _cpuCallback(std::move(other._cpuCallback))
        , _device(other._device)
    {
    }

    VulkanFence::~VulkanFence()
    {
        if (_semaphore)
        {
            vk::Device vkDevice = VulkanCast<vk::Device>(_device->GetNative());
            vkDevice.destroySemaphore(_semaphore);
        }
    }

    VulkanFence& VulkanFence::operator=(VulkanFence&& other) noexcept
    {
        if (this != &other)
        {
            _semaphore   = std::exchange(other._semaphore, nullptr);
            _fenceValue  = other._fenceValue;
            _isFree      = other._isFree;
            _cpuCallback = std::move(other._cpuCallback);
            _device      = other._device;
        }
        return *this;
    }

    void VulkanFence::Wait()
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(_device->GetNative());

        const vk::SemaphoreWaitInfo waitInfo =
        {
            .semaphoreCount = 1,
            .pSemaphores    = &_semaphore,
            .pValues        = &_fenceValue
        };

        const vk::Result waitResult = vkDevice.waitSemaphores(waitInfo, std::numeric_limits<std::uint64_t>::max());
        VK_CHECK(waitResult, "Failed to wait on timeline semaphore");

        if (_cpuCallback)
        {
            _cpuCallback();
        }
    }

    void* VulkanFence::GetNative() const
    {
        return VulkanNative(_semaphore);
    }
} // namespace rhi::vulkan
