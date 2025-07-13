#pragma once

#include "CommandList.h"

namespace dx12
{
    // Wrapper for an ID3D12QueryHeap to gather rendering statistics.
    class StatisticsQuery
    {
    public:
        StatisticsQuery();
        // Copy constructor.
        StatisticsQuery(const StatisticsQuery& other);
        // Move constructor.
        StatisticsQuery(StatisticsQuery&& other) noexcept;
        // Destructor.
        ~StatisticsQuery();

        // Copy assignment operator.
        StatisticsQuery& operator=(const StatisticsQuery& other);
        // Move assignment operator.
        StatisticsQuery& operator=(StatisticsQuery&& other) noexcept;

        // Create the query heap and an associated resource to store render statistics.
        void Create();

        // Start collecting rendering statistics.
        void BeginQuery(CommandList& commandList);
        // Stop collecting rendering statistics.
        void EndQuery(CommandList& commandList);

        // Resolve the statistics data gathered between BeginQuery and EndQuery calls.
        void ResolveQueryData(CommandList& commandList);
        // Retrieve the resolved rendering statistics.
        const D3D12_QUERY_DATA_PIPELINE_STATISTICS& GetStatistics();

    private:
        // Pointer to the DirectX 12 query heap used for statistics gathering.
        ComPtr<ID3D12QueryHeap> _statisticsQueryHeap;

        // Resource used to store query results.
        dx12::Resource _statisticsResource;
        // Pointer to the resolved statistics data stored in _statisticsResource.
        D3D12_QUERY_DATA_PIPELINE_STATISTICS* _statisticsData;
    };
} // namespace dx12
