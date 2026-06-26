#pragma once

#include "StatisticsQuery.h"

namespace rhi
{
    class Buffer;
    class CommandList;
    class QueryHeap;
} // namespace rhi

namespace rhi::vulkan
{
    class VulkanStatisticsQuery final : public rhi::StatisticsQuery
    {
    public:
        VulkanStatisticsQuery(const VulkanStatisticsQuery&) = delete;
        VulkanStatisticsQuery(VulkanStatisticsQuery&& other) noexcept;
        ~VulkanStatisticsQuery() override;

        VulkanStatisticsQuery& operator=(const VulkanStatisticsQuery&) = delete;
        VulkanStatisticsQuery& operator=(VulkanStatisticsQuery&& other) noexcept;

        void BeginQuery(rhi::CommandList* commandList) override;
        void EndQuery(rhi::CommandList* commandList) override;

        void ResolveQueryData(rhi::CommandList* commandList) override;
        const rhi::PipelineStatistics& GetStatistics() override;

    private:
        friend class VulkanDevice;

        VulkanStatisticsQuery(rhi::Device* device, const std::string& name = "");

        std::unique_ptr<rhi::QueryHeap> _queryHeap;

        std::shared_ptr<rhi::Buffer> _statisticsResource;
        rhi::PipelineStatistics _statistics;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
