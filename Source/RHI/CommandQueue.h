#pragma once

#include <vector>

namespace rhi
{
    class Fence;

    // CommandQueue is an abstract interface representing a command queue, which is responsible for executing command lists on the GPU
    class CommandQueue
    {
    public:
        CommandQueue() = default;
        CommandQueue(const CommandQueue&) = delete;
        CommandQueue(CommandQueue&&) noexcept = default;
        virtual ~CommandQueue() = default;

        CommandQueue& operator=(const CommandQueue&) = delete;
        CommandQueue& operator=(CommandQueue&&) noexcept = default;

        // Executes the specified command lists on the GPU, allowing the application to submit recorded commands for execution. 
        // The command lists may be processed asynchronously by the GPU, depending on the implementation of the command queue and the underlying graphics API
        virtual void ExecuteCommandLists(const std::vector<CommandList*>& commandLists) = 0;

        // Signals the fence with a value, allowing the application to synchronize GPU and CPU operations by indicating that a certain point in the command queue has been reached
        virtual void Signal(Fence* fence, uint64_t value) = 0;
        // Waits for the fence to reach a specific value, allowing the application to synchronize GPU and CPU operations by blocking until the GPU has reached the specified point in the command queue
        virtual void Wait(Fence* fence, uint64_t value) = 0;

        // Retrieves the timestamp frequency of the command queue, which indicates the number of GPU ticks per second and can be used for timing and performance measurements
        virtual std::uint64_t GetTimestampFrequency() const = 0;

        // Retrieves the type of the command queue, which indicates its intended usage and determines the appropriate command list types that can be executed on it
        virtual rhi::CommandListType GetType() const = 0;

        // Retrieves the native command queue object, allowing the application to access the underlying API-specific command queue
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
