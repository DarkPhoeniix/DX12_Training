#pragma once

#include "RHI/CommandListPool.h"

namespace rhi::vulkan
{
    class VulkanCommandListPool final : public rhi::CommandListPool
    {
    public:
        VulkanCommandListPool(rhi::Device* device);
        ~VulkanCommandListPool() override = default;

        rhi::CommandList* AllocateCommandList(rhi::CommandListType type) override;
        void FreeCommandList(rhi::CommandList* commandList) override;

    private:
        struct Executor
        {
            rhi::CommandList* CommandList;
            bool IsFree;
        };

        std::deque<Executor> _graphicsExecutors;
        std::deque<Executor> _computeExecutors;
        std::deque<Executor> _copyExecutors;

        std::deque<std::unique_ptr<rhi::CommandList>> _commandLists;

        rhi::Device* _device;
    };
} // namespace rhi::vulkan
