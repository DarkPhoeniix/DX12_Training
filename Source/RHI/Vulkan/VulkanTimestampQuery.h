#pragma once

#include "TimestampQuery.h"

namespace rhi
{
    class Buffer;
    class CommandList;
    class QueryHeap;
} // namespace rhi

namespace rhi::vulkan
{
    class VulkanTimestampQuery final : public rhi::TimestampQuery
    {
    public:
        VulkanTimestampQuery(const VulkanTimestampQuery&) = delete;
        VulkanTimestampQuery(VulkanTimestampQuery&& other) noexcept;
        ~VulkanTimestampQuery() override = default;

        VulkanTimestampQuery& operator=(const VulkanTimestampQuery&) = delete;
        VulkanTimestampQuery& operator=(VulkanTimestampQuery&& other) noexcept;

        void Begin(rhi::CommandList* commandList, std::uint32_t index) override;
        void End(rhi::CommandList* commandList, std::uint32_t index) override;

        void Resolve(rhi::CommandList* commandList, std::uint32_t numTimestamps, std::shared_ptr<rhi::Buffer> destination, std::uint64_t destinationOffset) override;

        std::uint64_t GetFrequency() const override;

    private:
        friend class VulkanDevice;

        VulkanTimestampQuery(rhi::Device* device, std::uint32_t timestampsCount, const std::string& name = "");

        std::unique_ptr<rhi::QueryHeap> _queryHeap;
        std::uint64_t _frequency;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
