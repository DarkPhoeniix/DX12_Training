#include "DX12LibPCH.h"

#include "StatisticsQuery.h"

namespace dx12
{
    StatisticsQuery::StatisticsQuery()
        : _statisticsQueryHeap(nullptr)
        , _statisticsResource(nullptr)
        , _statisticsData(nullptr)
    {
    }

    StatisticsQuery::StatisticsQuery(const StatisticsQuery& other)
        : _statisticsQueryHeap(other._statisticsQueryHeap)
        , _statisticsResource(other._statisticsResource)
        , _statisticsData(other._statisticsData)
    {
    }

    StatisticsQuery::StatisticsQuery(StatisticsQuery&& other) noexcept
        : _statisticsQueryHeap(std::move(other._statisticsQueryHeap))
        , _statisticsResource(std::move(other._statisticsResource))
        , _statisticsData(std::move(other._statisticsData))
    {
    }

    StatisticsQuery::~StatisticsQuery()
    {
        _statisticsQueryHeap = nullptr;
    }

    StatisticsQuery& StatisticsQuery::operator=(const StatisticsQuery& other)
    {
        if (this != &other)
        {
            _statisticsQueryHeap = other._statisticsQueryHeap;
            _statisticsResource = other._statisticsResource;
            _statisticsData = other._statisticsData;
        }

        return *this;
    }

    StatisticsQuery& StatisticsQuery::operator=(StatisticsQuery&& other) noexcept
    {
        if (this != &other)
        {
            _statisticsQueryHeap = std::move(other._statisticsQueryHeap);
            _statisticsResource = std::move(other._statisticsResource);
            _statisticsData = std::move(other._statisticsData);
        }

        return *this;
    }

    void StatisticsQuery::Create()
    {
        // Create the query heap
        D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
        {
            queryHeapDesc.Count = 1;
            queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
        }
        HRESULT result = Device::GetDXDevice()->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&_statisticsQueryHeap));
        CHECK(result, "Failed to create query heap for pipeline statistics.");

        // Create the resource for stats data
        ResourceDescription statisticsResourceDesc = {};
        {
            statisticsResourceDesc.SetDimension(D3D12_RESOURCE_DIMENSION_BUFFER);
            statisticsResourceDesc.SetSize({ sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS), 1 });
            statisticsResourceDesc.SetFormat(DXGI_FORMAT_UNKNOWN);
            statisticsResourceDesc.SetResourceType(ResourceType::Buffer | ResourceType::ReadBack);
        }
        _statisticsResource = ResourceFactory::Create("Statistcs query buffer", statisticsResourceDesc);
        _statisticsResource->CreateCommitedResource(dx12::ResourceState::CopyDest);
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
        commandList.ResolveQueryData(_statisticsQueryHeap, D3D12_QUERY_TYPE_PIPELINE_STATISTICS, 0, *_statisticsResource, 0);
    }

    const D3D12_QUERY_DATA_PIPELINE_STATISTICS& StatisticsQuery::GetStatistics()
    {
        _statisticsData = _statisticsResource->Map<D3D12_QUERY_DATA_PIPELINE_STATISTICS>();
        return *_statisticsData;
    }
} // namespace dx12
