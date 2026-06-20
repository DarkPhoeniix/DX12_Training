#pragma once

// Logs and breaks when a vk::Result is not eSuccess. Macros are not
// namespace-scoped, so this lives at file scope. Uses std::format placeholders
// ({}) to match logging::Logger, and vk::to_string to stringify the result.
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
