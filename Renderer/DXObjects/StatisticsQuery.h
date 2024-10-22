#pragma once

#include "DXObjects/CommandList.h"

namespace Core
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
        Core::Resource _statResource;

        D3D12_QUERY_DATA_PIPELINE_STATISTICS* _statisticsData;
    };
} // namespace Core
