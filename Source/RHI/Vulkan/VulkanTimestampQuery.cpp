
#include "RHI_PCH.h"

#include "VulkanTimestampQuery.h"

#include "CommandList.h"
#include "CommandQueue.h"
#include "QueryHeap.h"

namespace rhi::vulkan
{
    VulkanTimestampQuery::VulkanTimestampQuery(rhi::Device* device, std::uint32_t timestampsCount, const std::string& name)
        : _queryHeap(nullptr)
        , _frequency(device->GetGraphicsQueue()->GetTimestampFrequency())
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        const QueryHeapDescription description =
        {
            .Type = QueryHeapType::Timestamp,
            .Count = timestampsCount,
            .NodeMask = 0
        };
        _queryHeap = device->CreateQueryHeap(description, name);
    }

    VulkanTimestampQuery::VulkanTimestampQuery(VulkanTimestampQuery&& other) noexcept
        : _queryHeap(std::move(other._queryHeap))
        , _frequency(other._frequency)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanTimestampQuery& VulkanTimestampQuery::operator=(VulkanTimestampQuery&& other) noexcept
    {
        if (this != &other)
        {
            _queryHeap = std::move(other._queryHeap);
            _frequency = other._frequency;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    void VulkanTimestampQuery::Begin(rhi::CommandList* commandList, std::uint32_t index)
    {
        commandList->EndQuery(_queryHeap.get(), QueryType::Timestamp, index);
    }

    void VulkanTimestampQuery::End(rhi::CommandList* commandList, std::uint32_t index)
    {
        commandList->EndQuery(_queryHeap.get(), QueryType::Timestamp, index);
    }

    void VulkanTimestampQuery::Resolve(rhi::CommandList* commandList, std::uint32_t numTimestamps, std::shared_ptr<rhi::Buffer> destination, std::uint64_t destinationOffset)
    {
        commandList->ResolveQueryData(_queryHeap.get(), QueryType::Timestamp, 0, numTimestamps, destination, destinationOffset);
    }

    std::uint64_t VulkanTimestampQuery::GetFrequency() const
    {
        return _frequency;
    }
} // namespace rhi::vulkan
