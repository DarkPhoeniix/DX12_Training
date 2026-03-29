#pragma once

namespace rhi
{
    class Buffer;
    class CommandList;

    class TimestampQuery
    {
    public:
        TimestampQuery() = default;
        TimestampQuery(const TimestampQuery&) = delete;
        TimestampQuery(TimestampQuery&&) noexcept = default;
        virtual ~TimestampQuery() = default;

        TimestampQuery& operator=(const TimestampQuery&) = delete;
        TimestampQuery& operator=(TimestampQuery&&) noexcept = default;

        virtual void Begin(CommandList* commandList, std::uint32_t index) = 0;
        virtual void End(CommandList* commandList, std::uint32_t index) = 0;

        virtual void Resolve(CommandList* commandList, std::uint32_t numTimestamps, std::shared_ptr<Buffer> destination, std::uint64_t destinationOffset) = 0;

        virtual std::uint64_t GetFrequency() const = 0;
    };
} // namespace rhi
