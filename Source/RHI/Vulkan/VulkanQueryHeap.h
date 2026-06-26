#pragma once

#include "QueryHeap.h"
#include "VulkanHelpers.h"

namespace rhi
{
    class Device;
} // namespace rhi

namespace rhi::vulkan
{
    class VulkanQueryHeap final : public rhi::QueryHeap
    {
    public:
        VulkanQueryHeap(const VulkanQueryHeap&) = delete;
        VulkanQueryHeap(VulkanQueryHeap&& other) noexcept;
        ~VulkanQueryHeap() override;

        VulkanQueryHeap& operator=(const VulkanQueryHeap&) = delete;
        VulkanQueryHeap& operator=(VulkanQueryHeap&& other) noexcept;

        QueryHeapType GetType() const override;

        void* GetNative() const override;

    private:
        friend class VulkanDevice;

        VulkanQueryHeap(rhi::Device* device, const QueryHeapDescription& description, const std::string& name = "");

        vk::QueryPool     _queryPool;
        rhi::QueryHeapType _type;

        rhi::Device* _device;

#if ENABLE_DEBUG_NAMES
        std::string _name;
#endif // ENABLE_DEBUG_NAMES
    };
} // namespace rhi::vulkan
