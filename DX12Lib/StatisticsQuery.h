#pragma once

#include "CommandList.h"

namespace dx12
{
    class StatisticsQuery
    {
    public:
        StatisticsQuery();
        ~StatisticsQuery();

        void Create();

        void BeginQuery(CommandList& commandList);
        void EndQuery(CommandList& commandList);

        void ResolveQueryData(CommandList& commandList);
        const D3D12_QUERY_DATA_PIPELINE_STATISTICS& GetStatistics();

    private:
        ComPtr<ID3D12QueryHeap> _statQueryHeap;
        dx12::Resource _statResource;

        D3D12_QUERY_DATA_PIPELINE_STATISTICS* _statisticsData;
    };
} // namespace dx12
