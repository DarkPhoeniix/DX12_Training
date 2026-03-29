#pragma once

namespace rhi
{
    class CommandList;

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

    class StatisticsQuery
    {
    public:
        StatisticsQuery() = default;
        StatisticsQuery(const StatisticsQuery&) = delete;
        StatisticsQuery(StatisticsQuery&&) noexcept = default;
        virtual ~StatisticsQuery() = default;

        StatisticsQuery& operator=(const StatisticsQuery&) = delete;
        StatisticsQuery& operator=(StatisticsQuery&&) noexcept = default;

        virtual void BeginQuery(CommandList* commandList) = 0;
        virtual void EndQuery(CommandList* commandList) = 0;

        virtual void ResolveQueryData(CommandList* commandList) = 0;
        virtual const PipelineStatistics& GetStatistics() = 0;
    };
} // namespace rhi
