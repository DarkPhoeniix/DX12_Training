#pragma once

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
} // namespace rhi::vulkan
