
#include "RHI_PCH.h"

#include "D3D12StatisticsQuery.h"

#include "D3D12Helpers.h"

#include "Buffer.h"
#include "QueryHeap.h"

namespace rhi::d3d12
{
    D3D12StatisticsQuery::D3D12StatisticsQuery(rhi::Device* device, const std::string& name)
        : _statisticsResource(nullptr)
        , _statistics()
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        rhi::QueryHeapDescription heapDescription =
        {
            .Type = rhi::QueryHeapType::PipelineStatistics,
            .Count = 1,
            .NodeMask = 0
        };
        _queryHeap = device->CreateQueryHeap(heapDescription);

        rhi::BufferDescription bufferDescription =
        {
            .Size = sizeof(rhi::PipelineStatistics),
            .Stride = 0,
            .Format = rhi::Format::UNKNOWN,
            .Usage = rhi::ResourceUsage::Readback,
            .Flags = rhi::ResourceFlags::None
        };
        _statisticsResource = device->CreateBuffer(bufferDescription);
    }

    D3D12StatisticsQuery::D3D12StatisticsQuery(D3D12StatisticsQuery&& other) noexcept
        : _queryHeap(std::move(other._queryHeap))
        , _statisticsResource(std::move(other._statisticsResource))
        , _statistics(std::move(other._statistics))
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    D3D12StatisticsQuery::~D3D12StatisticsQuery()
    {
    }

    D3D12StatisticsQuery& D3D12StatisticsQuery::operator=(D3D12StatisticsQuery&& other) noexcept
    {
        if (this != &other)
        {
            _queryHeap = std::move(other._queryHeap);
            _statisticsResource = std::move(other._statisticsResource);
            _statistics = std::move(other._statistics);
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }

        return *this;
    }

    void D3D12StatisticsQuery::BeginQuery(CommandList& commandList)
    {
        commandList.BeginQuery(*_queryHeap, rhi::QueryType::PipelineStatistics, 0);
    }

    void D3D12StatisticsQuery::EndQuery(CommandList& commandList)
    {
        commandList.EndQuery(*_queryHeap, rhi::QueryType::PipelineStatistics, 0);
    }

    void D3D12StatisticsQuery::ResolveQueryData(CommandList& commandList)
    {
        commandList.ResolveQueryData(*_queryHeap, rhi::QueryType::PipelineStatistics, 0, _statisticsResource, 0);
    }

    const rhi::PipelineStatistics& D3D12StatisticsQuery::GetStatistics()
    {
        _statistics = *_statisticsResource->Map<rhi::PipelineStatistics>();
        return _statistics;
    }
} // namespace rhi::d3d12
