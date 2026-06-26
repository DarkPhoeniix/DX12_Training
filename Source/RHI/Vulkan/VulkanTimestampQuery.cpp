
#include "RHI_PCH.h"

#include "VulkanTimestampQuery.h"

namespace rhi::vulkan
{
    VulkanTimestampQuery::VulkanTimestampQuery(rhi::Device* device, std::uint32_t timestampsCount, const std::string& name)
        : _queryHeap(nullptr)
        , _frequency(0)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        NOT_IMPLEMENTED();
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

    void VulkanTimestampQuery::Begin(rhi::CommandList*, std::uint32_t)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanTimestampQuery::End(rhi::CommandList*, std::uint32_t)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanTimestampQuery::Resolve(rhi::CommandList*, std::uint32_t, std::shared_ptr<rhi::Buffer>, std::uint64_t)
    {
        NOT_IMPLEMENTED();
    }

    std::uint64_t VulkanTimestampQuery::GetFrequency() const
    {
        return _frequency;
    }
} // namespace rhi::vulkan
