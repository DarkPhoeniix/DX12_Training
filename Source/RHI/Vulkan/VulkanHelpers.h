#pragma once

// VMA opaque handle forward declarations — avoids including vk_mem_alloc.h in headers
struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

#define VK_CHECK(result, message)                                                       \
    do {                                                                                \
        if ((result) != vk::Result::eSuccess)                                           \
        {                                                                               \
            logging::Logger::Instance().Log(logging::Level::Error,                      \
                "[{} ({})] {} (VkResult: {})",                                          \
                __func__, __LINE__, (message), vk::to_string(result));                  \
            __debugbreak();                                                             \
        }                                                                               \
    } while (0)

namespace rhi::vulkan
{
    template<typename T>
    concept VulkanHandle = requires { typename T::NativeType; };

    template<VulkanHandle T>
    inline T VulkanCast(void* native)
    {
        ASSERT(native != nullptr, "Trying to cast a null pointer.");
        return T(reinterpret_cast<typename T::NativeType>(native));
    }

    template<VulkanHandle T>
    inline void* VulkanNative(T handle)
    {
        return reinterpret_cast<void*>(static_cast<typename T::NativeType>(handle));
    }

    template<VulkanHandle T>
    inline void SetVulkanName([[maybe_unused]] vk::Device device, [[maybe_unused]] T handle, [[maybe_unused]] const std::string& name)
    {
#if ENABLE_DEBUG_NAMES
        if (handle && !name.empty())
        {
            const vk::DebugUtilsObjectNameInfoEXT nameInfo =
            {
                .objectType   = T::objectType,
                .objectHandle = reinterpret_cast<std::uint64_t>(static_cast<typename T::NativeType>(handle)),
                .pObjectName  = name.c_str()
            };
            vk::Result result = device.setDebugUtilsObjectNameEXT(nameInfo);
            VK_CHECK(result, "Failed to set debug name for Vulkan object");
        }
#endif // ENABLE_DEBUG_NAMES
    }
} // namespace rhi::vulkan
