#pragma once

namespace rhi
{
    enum class QueryHeapType : std::uint8_t
    {
        Occlusion,
        Timestamp,
        PipelineStatistics,
        PipelineStatistics1
    };

    struct QueryHeapDescription
    {
        QueryHeapType Type;
        std::uint32_t Count;
        std::uint32_t NodeMask;
    };

    class QueryHeap
    {
    public:
        QueryHeap() = default;
        QueryHeap(const QueryHeap&) = delete;
        QueryHeap(QueryHeap&&) noexcept = default;
        virtual ~QueryHeap() = default;

        QueryHeap& operator=(const QueryHeap&) = delete;
        QueryHeap& operator=(QueryHeap&&) noexcept = default;

        virtual QueryHeapType GetType() const = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
