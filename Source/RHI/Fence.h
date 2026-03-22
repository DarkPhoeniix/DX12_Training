#pragma once

namespace rhi
{
    class Fence
    {
    public:
        Fence() = default;
        Fence(const Fence&) = delete;
        Fence(Fence&&) noexcept = default;
        virtual ~Fence() = default;

        Fence& operator=(const Fence&) = delete;
        Fence& operator=(Fence&&) noexcept = default;

        virtual void Wait() = 0;

        virtual void SetValue(std::uint64_t value) = 0;
        virtual std::uint64_t GetValue() const = 0;

        virtual void SetFree(bool isFree) = 0;
        virtual bool IsFree() const = 0;

        virtual void SetCompletionCallback(const std::function<void()>& callback) = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
