#pragma once

namespace rhi
{
    // Fence is an abstract interface representing a synchronization primitive used for GPU-CPU synchronization. 
    // It allows the application to wait for specific points in the GPU command execution and to signal when certain operations have completed
    class Fence
    {
    public:
        Fence() = default;
        Fence(const Fence&) = delete;
        Fence(Fence&&) noexcept = default;
        virtual ~Fence() = default;

        Fence& operator=(const Fence&) = delete;
        Fence& operator=(Fence&&) noexcept = default;

        // Waits for the fence to reach a specific value, blocking the CPU until the GPU has reached the specified point in the command queue
        virtual void Wait() = 0;

        // Sets the value of the fence, allowing the application to signal that a certain point in the command queue has been reached.
        // The GPU will update the fence value as it executes commands, and the CPU can wait for specific values to synchronize operations
        virtual void SetValue(std::uint64_t value) = 0;
        // Retrieves the current value of the fence, which indicates the latest point in the command queue that has been reached by the GPU
        virtual std::uint64_t GetValue() const = 0;

        // Sets the free state of the fence, allowing the application to indicate whether the fence is currently free or in use for synchronization
        virtual void SetFree(bool isFree) = 0;
        // Indicates whether the fence is currently free, meaning that it is not being used for synchronization and can be reused for new operations
        virtual bool IsFree() const = 0;

        // Sets a completion callback function that will be called when the fence reaches a specific value.
        // Allows the application to perform actions or trigger events when certain GPU operations have completed
        virtual void SetCompletionCallback(const std::function<void()>& callback) = 0;

        // Retrieves the native fence object, allowing the application to access the underlying API-specific fence
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
