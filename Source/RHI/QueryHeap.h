#pragma once

namespace rhi
{
    // QueryHeapType represents the type of query heap, which determines the type of data that can be collected when using query heaps for data collection
    enum class QueryHeapType : std::uint8_t
    {
        Occlusion,
        Timestamp,
        PipelineStatistics,
        PipelineStatistics1
    };

    // QueryHeapDescription encapsulates the properties and configuration of a query heap
    struct QueryHeapDescription
    {
        QueryHeapType Type;
        std::uint32_t Count;
        std::uint32_t NodeMask;
    };

    // QueryHeap is an abstract interface representing a query heap, which is a collection of queries used for data collection on the GPU
    class QueryHeap
    {
    public:
        QueryHeap() = default;
        QueryHeap(const QueryHeap&) = delete;
        QueryHeap(QueryHeap&&) noexcept = default;
        virtual ~QueryHeap() = default;

        QueryHeap& operator=(const QueryHeap&) = delete;
        QueryHeap& operator=(QueryHeap&&) noexcept = default;

        // Retrieves the type of the query heap, which indicates the type of data that can be collected
        virtual QueryHeapType GetType() const = 0;

        // Retrieves the native query heap object, allowing the application to access the underlying API-specific query heap
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
