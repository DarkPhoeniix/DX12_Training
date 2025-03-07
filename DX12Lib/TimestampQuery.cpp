#include "DX12LibPCH.h"

#include "TimestampQuery.h"

namespace dx12
{
    void TimestampQuery::Create(std::uint32_t numTimers)
    {
        _numTimers = numTimers;

        // Create the query heap
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        {
            queryHeapDesc.Count = 1;
            queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        }
        Device::GetDXDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_timestampQueryHeap));

        // Create the resource for stats data
        ResourceDescription statisticsResourceDesc = {};
        {
            statisticsResourceDesc.SetDimension(D3D12_RESOURCE_DIMENSION_BUFFER);
            statisticsResourceDesc.SetSize({ sizeof(std::uint64_t) * _numTimers, 1 });
            statisticsResourceDesc.SetFormat(DXGI_FORMAT_UNKNOWN);
            statisticsResourceDesc.SetResourceType(ResourceType::Buffer | ResourceType::ReadBack);
            _statisticsResource.SetResourceDescription(statisticsResourceDesc);
        }
        _statisticsResource.CreateCommitedResource();
    }

    void TimestampQuery::QueryTimestamp(CommandList& commandList, std::uint32_t timerId)
    {
        commandList.EndQuery(_timestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, timerId);
    }

    void TimestampQuery::ResolveQueryData(CommandList& commandList)
    {
        for (std::uint32_t index = 0; index < _numTimers; ++index)
        {
            commandList.ResolveQueryData(_timestampQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, index, _statisticsResource, index * sizeof(std::uint32_t));
        }
    }

    std::uint64_t TimestampQuery::GetStatistics(std::uint32_t timerId)
    {
        _timeData = (std::uint64_t*)_statisticsResource.Map();
        return _timeData[timerId];
    }
} // namespace dx12
