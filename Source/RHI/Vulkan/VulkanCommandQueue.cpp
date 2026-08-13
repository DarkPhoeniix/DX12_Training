
#include "RHI_PCH.h"

#include "VulkanCommandQueue.h"

#include "VulkanDevice.h"
#include "VulkanHelpers.h"
#include "VulkanSwapChain.h"

#include "CommandList.h"
#include "Fence.h"

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
        std::vector<vk::CommandBufferSubmitInfo> bufferInfos;
        bufferInfos.reserve(commandLists.size());
        for (rhi::CommandList* commandList : commandLists)
        {
            bufferInfos.push_back({ .commandBuffer = VulkanCast<vk::CommandBuffer>(commandList->GetNative()) });
        }

        auto* swapChain = static_cast<VulkanSwapChain*>(static_cast<VulkanDevice*>(_device)->GetSwapChain());   // TODO: fix dependencies here?

        std::vector<vk::SemaphoreSubmitInfo> waitInfos;
        if (swapChain)
        {
            if (vk::Semaphore acquireSemaphore = swapChain->ConsumeAcquireSemaphore())
            {
                waitInfos.push_back(
                {
                    .semaphore = acquireSemaphore,
                    .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput
                });
            }
        }

        std::vector<vk::SemaphoreSubmitInfo> signalInfos;
        if (swapChain)
        {
            signalInfos.push_back(
            {
                .semaphore = swapChain->GetRenderFinishedSemaphore(),
                .stageMask = vk::PipelineStageFlagBits2::eAllCommands
            });
        }

        const vk::SubmitInfo2 submitInfo =
        {
            .waitSemaphoreInfoCount = static_cast<std::uint32_t>(waitInfos.size()),
            .pWaitSemaphoreInfos = waitInfos.data(),
            .commandBufferInfoCount = static_cast<std::uint32_t>(bufferInfos.size()),
            .pCommandBufferInfos = bufferInfos.data(),
            .signalSemaphoreInfoCount = static_cast<std::uint32_t>(signalInfos.size()),
            .pSignalSemaphoreInfos = signalInfos.data()
        };

        const vk::Result submitResult = _queue.submit2(submitInfo);
        VK_CHECK(submitResult, "Failed to submit command lists");
    }

    void VulkanCommandQueue::Signal(rhi::Fence* fence, std::uint64_t value)
    {
        const vk::SemaphoreSubmitInfo signalInfo =
        {
            .semaphore = VulkanCast<vk::Semaphore>(fence->GetNative()),
            .value = value,
            .stageMask = vk::PipelineStageFlagBits2::eAllCommands
        };

        const vk::SubmitInfo2 submitInfo =
        {
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &signalInfo
        };

        const vk::Result submitResult = _queue.submit2(submitInfo);
        VK_CHECK(submitResult, "Failed to signal fence");
    }

    void VulkanCommandQueue::Wait(rhi::Fence* fence, std::uint64_t value)
    {
        const vk::SemaphoreSubmitInfo waitInfo =
        {
            .semaphore = VulkanCast<vk::Semaphore>(fence->GetNative()),
            .value = value,
            .stageMask = vk::PipelineStageFlagBits2::eAllCommands
        };

        const vk::SubmitInfo2 submitInfo =
        {
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &waitInfo
        };

        const vk::Result submitResult = _queue.submit2(submitInfo);
        VK_CHECK(submitResult, "Failed to wait on fence");
    }

    std::uint64_t VulkanCommandQueue::GetTimestampFrequency() const
    {
        const vk::PhysicalDeviceLimits& limits = static_cast<VulkanDevice*>(_device)->GetPhysicalDevice().getProperties().limits;
        ASSERT(limits.timestampPeriod > 0.0f, "Device does not support timestamp queries.");

        return static_cast<std::uint64_t>(1.0 / (limits.timestampPeriod * 1e-9));
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
