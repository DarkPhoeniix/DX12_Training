#include "DX12LibPCH.h"

#include "StatisticsQuery.h"

namespace dx12
{
    void StatisticsQuery::Create()
    {
        // Create the query heap
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        {
            queryHeapDesc.Count = 1;
            queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        }
        Device::GetDXDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_statisticsQueryHeap));

        // Create the resource for stats data
        ResourceDescription statisticsResourceDesc = {};
        {
            statisticsResourceDesc.SetDimension(D3D12_RESOURCE_DIMENSION_BUFFER);
            statisticsResourceDesc.SetSize({ sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS), 1 });
            statisticsResourceDesc.SetFormat(DXGI_FORMAT_UNKNOWN);
            statisticsResourceDesc.SetResourceType(ResourceType::Buffer | ResourceType::ReadBack);
            _statisticsResource.SetResourceDescription(statisticsResourceDesc);
        }
        _statisticsResource.CreateCommitedResource();
    }

    void StatisticsQuery::BeginQuery(CommandList& commandList)
    {
        commandList.BeginQuery(_statisticsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
    }

    void StatisticsQuery::EndQuery(CommandList& commandList)
    {
        commandList.EndQuery(_statisticsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0);
    }

    void StatisticsQuery::ResolveQueryData(CommandList& commandList)
    {
        commandList.ResolveQueryData(_statisticsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, _statisticsResource, 0);
    }

    const D3D12_QUERY_DATA_PIPELINE_STATISTICS& StatisticsQuery::GetStatistics()
    {
        _statisticsData = (D3D12_QUERY_DATA_PIPELINE_STATISTICS*)_statisticsResource.Map();
        return *_statisticsData;
    }
} // namespace dx12
