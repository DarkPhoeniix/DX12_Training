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
    class D3D12StatisticsQuery final : public StatisticsQuery
    {
    public:
        D3D12StatisticsQuery(const D3D12StatisticsQuery& other) = delete;
        D3D12StatisticsQuery(D3D12StatisticsQuery&& other) noexcept;
        ~D3D12StatisticsQuery() override;

        D3D12StatisticsQuery& operator=(const D3D12StatisticsQuery& other) = delete;
        D3D12StatisticsQuery& operator=(D3D12StatisticsQuery&& other) noexcept;

        void BeginQuery(CommandList* commandList) override;
        void EndQuery(CommandList* commandList) override;

        void ResolveQueryData(CommandList* commandList) override;
        const PipelineStatistics& GetStatistics() override;

    private:
        friend class D3D12Device;

        D3D12StatisticsQuery(Device* device, const std::string& name = "");

        std::unique_ptr<QueryHeap> _queryHeap;

        std::shared_ptr<Buffer> _statisticsResource;
        PipelineStatistics _statistics;

        std::string _name;
    };
} // namespace rhi::d3d12
