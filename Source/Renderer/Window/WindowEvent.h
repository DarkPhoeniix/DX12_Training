#pragma once

#include <cstdint>

namespace core
{
    struct WindowEvent
    {
        void*           WindowHandle;
        std::uint32_t   Message;
        std::uint64_t   WParam;
        std::int64_t    LParam;
    };
} // namespace core
