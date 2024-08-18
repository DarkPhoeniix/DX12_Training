#pragma once

#include "DXObjects/GraphicsCommandList.h"

namespace Core
{
    class StatisticsQuery
    {
    public:
        StatisticsQuery();
        ~StatisticsQuery();

        void Create();

        void BeginQuery(GraphicsCommandList& commandList);
        void EndQuery(GraphicsCommandList& commandList);

        void ResolveQueryData(GraphicsCommandList& commandList);
        D3D12_QUERY_DATA_PIPELINE_STATISTICS GetStatistics();

    private:
        ComPtr<ID3D12Device2> _DXDevice;

        ComPtr<ID3D12QueryHeap> _statQueryHeap;
        Core::Resource _statResource;
    };
} // namespace Core
