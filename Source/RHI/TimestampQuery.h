#pragma once

namespace rhi
{
    class Buffer;
    class CommandList;

    // TimestampQuery is an abstract interface representing a timestamp query, which allows the application to measure GPU execution time by recording timestamps 
    // at specific points in the command list and retrieving the results
    class TimestampQuery
    {
    public:
        TimestampQuery() = default;
        TimestampQuery(const TimestampQuery&) = delete;
        TimestampQuery(TimestampQuery&&) noexcept = default;
        virtual ~TimestampQuery() = default;

        TimestampQuery& operator=(const TimestampQuery&) = delete;
        TimestampQuery& operator=(TimestampQuery&&) noexcept = default;

        // Begins a timestamp query on the GPU, allowing the application to record a timestamp at a specific point in the command list
        virtual void Begin(CommandList* commandList, std::uint32_t index) = 0;
        // Ends a timestamp query on the GPU, allowing the application to record a timestamp at a specific point in the command list and complete the query
        virtual void End(CommandList* commandList, std::uint32_t index) = 0;

        // Resolves the results of the timestamp query from the GPU, allowing the application to retrieve the recorded timestamps and store them in a destination buffer
        virtual void Resolve(CommandList* commandList, std::uint32_t numTimestamps, std::shared_ptr<Buffer> destination, std::uint64_t destinationOffset) = 0;

        // Retrieves the timestamp frequency of the GPU, which indicates the number of GPU ticks per second and can be used to convert timestamp values into time measurements
        virtual std::uint64_t GetFrequency() const = 0;
    };
} // namespace rhi
