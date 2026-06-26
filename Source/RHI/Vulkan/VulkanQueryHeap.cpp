
#include "RHI_PCH.h"

#include "VulkanQueryHeap.h"

namespace rhi::vulkan
{
    VulkanQueryHeap::VulkanQueryHeap(rhi::Device* device, const QueryHeapDescription& description, const std::string& name)
        : _queryPool(nullptr)
        , _type(description.Type)
        , _device(device)
#if ENABLE_DEBUG_NAMES
        , _name(name)
#endif // ENABLE_DEBUG_NAMES
    {
        NOT_IMPLEMENTED();
    }

    VulkanQueryHeap::VulkanQueryHeap(VulkanQueryHeap&& other) noexcept
        : _queryPool(std::exchange(other._queryPool, nullptr))
        , _type(other._type)
        , _device(other._device)
#if ENABLE_DEBUG_NAMES
        , _name(std::move(other._name))
#endif // ENABLE_DEBUG_NAMES
    {
    }

    VulkanQueryHeap::~VulkanQueryHeap()
    {
        NOT_IMPLEMENTED();
    }

    VulkanQueryHeap& VulkanQueryHeap::operator=(VulkanQueryHeap&& other) noexcept
    {
        if (this != &other)
        {
            _queryPool = std::exchange(other._queryPool, nullptr);
            _type      = other._type;
            _device    = other._device;
#if ENABLE_DEBUG_NAMES
            _name = std::move(other._name);
#endif // ENABLE_DEBUG_NAMES
        }
        return *this;
    }

    QueryHeapType VulkanQueryHeap::GetType() const
    {
        return _type;
    }

    void* VulkanQueryHeap::GetNative() const
    {
        return VulkanNative(_queryPool);
    }
} // namespace rhi::vulkan
