#pragma once

#include <vector>

namespace rhi
{
    class Fence;

    class CommandQueue
    {
    public:
        CommandQueue() = default;
        CommandQueue(const CommandQueue&) = delete;
        CommandQueue(CommandQueue&&) noexcept = default;
        virtual ~CommandQueue() = default;

        CommandQueue& operator=(const CommandQueue&) = delete;
        CommandQueue& operator=(CommandQueue&&) noexcept = default;

        virtual void ExecuteCommandLists(std::vector<CommandList*> commandLists) = 0;

        virtual void Signal(Fence* fence, uint64_t value) = 0;
        virtual void Wait(Fence* fence, uint64_t value) = 0;

        virtual std::uint64_t GetTimestampFrequency() const = 0;

        virtual rhi::CommandListType GetType() const = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
