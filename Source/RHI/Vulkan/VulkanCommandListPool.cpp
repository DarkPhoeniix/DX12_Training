
#include "RHI_PCH.h"

#include "VulkanCommandListPool.h"

namespace rhi::vulkan
{
    VulkanCommandListPool::VulkanCommandListPool(rhi::Device* device)
        : _device(device)
    {
    }

    rhi::CommandList* VulkanCommandListPool::AllocateCommandList(rhi::CommandListType type)
    {
        // TODO: use shared vk::CommandPool for optimization

        std::deque<Executor>* executors = nullptr;

        switch (type)
        {
        case rhi::CommandListType::Graphics: executors = &_graphicsExecutors; break;
        case rhi::CommandListType::Compute:  executors = &_computeExecutors;  break;
        case rhi::CommandListType::Copy:     executors = &_copyExecutors;     break;
        default:
            UNREACHABLE("Unsupported command list type.");
            return nullptr;
        }

        for (auto& e : *executors)
        {
            if (e.IsFree)
            {
                e.IsFree = false;
                return e.CommandList;
            }
        }

        const auto& newList = _commandLists.emplace_back(_device->CreateCommandList(type));
        auto& newExecutor = executors->emplace_back(newList.get(), false);
        return newExecutor.CommandList;
    }

    void VulkanCommandListPool::FreeCommandList(rhi::CommandList* commandList)
    {
        std::deque<Executor>* executors = nullptr;

        switch (commandList->GetCommandListType())
        {
        case rhi::CommandListType::Graphics: executors = &_graphicsExecutors; break;
        case rhi::CommandListType::Compute:  executors = &_computeExecutors;  break;
        case rhi::CommandListType::Copy:     executors = &_copyExecutors;     break;
        default:
            UNREACHABLE("Unsupported command list type.");
            return;
        }

        for (auto& e : *executors)
        {
            if (e.CommandList == commandList)
            {
                e.IsFree = true;
                return;
            }
        }

        LOG_ERROR("Command list not found in pool.");
    }
} // namespace rhi::vulkan
