
#include "RHI_PCH.h"

#include "VulkanCommandQueue.h"

#include "VulkanDevice.h"
#include "VulkanHelpers.h"

namespace rhi::vulkan
{
    VulkanCommandQueue::VulkanCommandQueue(rhi::Device* device, rhi::CommandListType type, std::uint32_t queueFamilyIndex)
        : _device(device)
        , _queue(nullptr)
        , _type(type)
        , _queueFamilyIndex(queueFamilyIndex)
    {
        vk::Device vkDevice = VulkanCast<vk::Device>(_device->GetNative());
        _queue = vkDevice.getQueue(_queueFamilyIndex, 0);
    }

    VulkanCommandQueue::VulkanCommandQueue(VulkanCommandQueue&& other) noexcept
        : _device(std::move(other._device))
        , _queue(other._queue)
        , _type(other._type)
        , _queueFamilyIndex(other._queueFamilyIndex)
    {
    }

    VulkanCommandQueue::~VulkanCommandQueue()
    {
    }

    VulkanCommandQueue& VulkanCommandQueue::operator=(VulkanCommandQueue&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _queue = other._queue;
            _type = other._type;
            _queueFamilyIndex = other._queueFamilyIndex;
        }

        return *this;
    }

    void VulkanCommandQueue::ExecuteCommandLists(const std::vector<rhi::CommandList*>& commandLists)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanCommandQueue::Signal(rhi::Fence* fence, std::uint64_t value)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanCommandQueue::Wait(rhi::Fence* fence, std::uint64_t value)
    {
        NOT_IMPLEMENTED();
    }

    std::uint64_t VulkanCommandQueue::GetTimestampFrequency() const
    {
        NOT_IMPLEMENTED();
        return std::uint64_t();
    }

    rhi::CommandListType VulkanCommandQueue::GetType() const
    {
        return _type;
    }

    std::uint32_t VulkanCommandQueue::GetQueueFamilyIndex() const
    {
        return _queueFamilyIndex;
    }

    void* VulkanCommandQueue::GetNative() const
    {
        return VulkanNative(_queue);
    }
} // namespace rhi::vulkan
