
#include "RHI_PCH.h"

#include "VulkanStatisticsQuery.h"

namespace rhi::vulkan
{
    VulkanStatisticsQuery::VulkanStatisticsQuery(rhi::Device* device, const std::string& name)
        : _queryHeap(nullptr)
        , _statisticsResource(nullptr)
        , _statistics{}
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        NOT_IMPLEMENTED();
    }

    VulkanStatisticsQuery::VulkanStatisticsQuery(VulkanStatisticsQuery&& other) noexcept
        : _queryHeap(std::move(other._queryHeap))
        , _statisticsResource(std::move(other._statisticsResource))
        , _statistics(other._statistics)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanStatisticsQuery::~VulkanStatisticsQuery()
    {
        NOT_IMPLEMENTED();
    }

    VulkanStatisticsQuery& VulkanStatisticsQuery::operator=(VulkanStatisticsQuery&& other) noexcept
    {
        if (this != &other)
        {
            _queryHeap          = std::move(other._queryHeap);
            _statisticsResource = std::move(other._statisticsResource);
            _statistics         = other._statistics;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    void VulkanStatisticsQuery::BeginQuery(rhi::CommandList*)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanStatisticsQuery::EndQuery(rhi::CommandList*)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanStatisticsQuery::ResolveQueryData(rhi::CommandList*)
    {
        NOT_IMPLEMENTED();
    }

    const rhi::PipelineStatistics& VulkanStatisticsQuery::GetStatistics()
    {
        return _statistics;
    }
} // namespace rhi::vulkan
