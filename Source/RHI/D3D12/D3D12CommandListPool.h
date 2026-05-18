#pragma once

#include "CommandListPool.h"

namespace rhi::d3d12
{
    class D3D12CommandListPool final : public CommandListPool
    {
    public:
        D3D12CommandListPool(Device* device);
        ~D3D12CommandListPool() override = default;

        CommandList* AllocateCommandList(CommandListType type) override;
        void FreeCommandList(CommandList* commandList) override;

    private:
        struct Executor
        {
            CommandList* CommandList;
            bool IsFree;
        };

        std::deque<Executor> _graphicsExecutors;
        std::deque<Executor> _computeExecutors;
        std::deque<Executor> _copyExecutors;

        std::deque<std::unique_ptr<CommandList>> _commandLists;

        Device* _device;
    };
} // namespace rhi::d3d12
