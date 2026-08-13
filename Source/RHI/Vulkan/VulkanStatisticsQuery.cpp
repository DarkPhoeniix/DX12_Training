
#include "RHI_PCH.h"

#include "VulkanStatisticsQuery.h"

#include "Buffer.h"
#include "CommandList.h"
#include "QueryHeap.h"

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
        const QueryHeapDescription heapDescription =
        {
            .Type = QueryHeapType::PipelineStatistics,
            .Count = 1,
            .NodeMask = 0
        };
        _queryHeap = device->CreateQueryHeap(heapDescription, name);

        const BufferDescription bufferDescription =
        {
            .Size = sizeof(PipelineStatistics),
            .Stride = 0,
            .Format = Format::UNKNOWN,
            .Usage = ResourceUsage::Readback,
            .Flags = ResourceFlags::None
        };
        _statisticsResource = device->CreateBuffer(bufferDescription, ResourceState::CopyDest, name);
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

    void VulkanStatisticsQuery::BeginQuery(rhi::CommandList* commandList)
    {
        commandList->BeginQuery(_queryHeap.get(), QueryType::PipelineStatistics, 0);
    }

    void VulkanStatisticsQuery::EndQuery(rhi::CommandList* commandList)
    {
        commandList->EndQuery(_queryHeap.get(), QueryType::PipelineStatistics, 0);
    }

    void VulkanStatisticsQuery::ResolveQueryData(rhi::CommandList* commandList)
    {
        commandList->ResolveQueryData(_queryHeap.get(), QueryType::PipelineStatistics, 0, _statisticsResource, 0);
    }

    const rhi::PipelineStatistics& VulkanStatisticsQuery::GetStatistics()
    {
        _statistics = *_statisticsResource->Map<PipelineStatistics>();
        _statisticsResource->Unmap();

        return _statistics;
    }
} // namespace rhi::vulkan
