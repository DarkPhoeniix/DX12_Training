#pragma once

#include "StatisticsQuery.h"

namespace rhi
{
    class Buffer;
    class CommandList;
    class QueryHeap;
} // namespace rhi

namespace rhi::d3d12
{
    // Wrapper for an ID3D12QueryHeap to gather rendering statistics.
    class D3D12StatisticsQuery final : public rhi::StatisticsQuery
    {
    public:
        // Copy constructor.
        D3D12StatisticsQuery(const D3D12StatisticsQuery& other) = delete;
        // Move constructor.
        D3D12StatisticsQuery(D3D12StatisticsQuery&& other) noexcept;
        // Destructor.
        ~D3D12StatisticsQuery() override;

        // Copy assignment operator.
        D3D12StatisticsQuery& operator=(const D3D12StatisticsQuery& other) = delete;
        // Move assignment operator.
        D3D12StatisticsQuery& operator=(D3D12StatisticsQuery&& other) noexcept;

        // Start collecting rendering statistics.
        void BeginQuery(rhi::CommandList& commandList) override;
        // Stop collecting rendering statistics.
        void EndQuery(rhi::CommandList& commandList) override;

        // Resolve the statistics data gathered between BeginQuery and EndQuery calls.
        void ResolveQueryData(rhi::CommandList& commandList) override;
        // Retrieve the resolved rendering statistics.
        const rhi::PipelineStatistics& GetStatistics() override;

    private:
        friend class D3D12Device;

        D3D12StatisticsQuery(rhi::Device* device, const std::string& name = "");

        // Pointer to the DirectX 12 query heap used for statistics gathering.
        std::unique_ptr<rhi::QueryHeap> _queryHeap;

        // Resource used to store query results.
        std::shared_ptr<rhi::Buffer> _statisticsResource;
        PipelineStatistics _statistics;

        std::string _name;
    };
} // namespace rhi::d3d12
