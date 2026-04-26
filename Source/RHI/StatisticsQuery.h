#pragma once

namespace rhi
{
    class CommandList;

    // PipelineStatistics encapsulates various metrics related to the GPU pipeline stages
    struct PipelineStatistics
    {
        std::uint64_t IAVertices;
        std::uint64_t IAPrimitives;
        std::uint64_t VSInvocations;
        std::uint64_t GSInvocations;
        std::uint64_t GSPrimitives;
        std::uint64_t CInvocations;
        std::uint64_t CPrimitives;
        std::uint64_t PSInvocations;
        std::uint64_t HSInvocations;
        std::uint64_t DSInvocations;
        std::uint64_t CSInvocations;
    };

    // StatisticsQuery is an abstract interface representing a statistics query, which can be used to gather statistics data and other metrics from the GPU
    class StatisticsQuery
    {
    public:
        StatisticsQuery() = default;
        StatisticsQuery(const StatisticsQuery&) = delete;
        StatisticsQuery(StatisticsQuery&&) noexcept = default;
        virtual ~StatisticsQuery() = default;

        StatisticsQuery& operator=(const StatisticsQuery&) = delete;
        StatisticsQuery& operator=(StatisticsQuery&&) noexcept = default;

        // Begins the statistics query on the GPU using the specified command list
        virtual void BeginQuery(CommandList* commandList) = 0;
        // Ends the statistics query on the GPU using the specified command list
        virtual void EndQuery(CommandList* commandList) = 0;

        // Resolves the statistics query data from the GPU using the specified command list, allowing the application to retrieve the collected statistics data
        virtual void ResolveQueryData(CommandList* commandList) = 0;
        // Retrieves the collected statistics data from the query, allowing the application to access the various metrics related to the GPU pipeline stages
        virtual const PipelineStatistics& GetStatistics() = 0;
    };
} // namespace rhi
