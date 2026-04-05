#pragma once

#include "CommandList.h"

#if ENABLE_GPU_EVENTS
    #define CONCAT_IMPL(x, y) x##y
    #define CONCAT(x, y) CONCAT_IMPL(x, y)
    #define GPU_SCOPED_EVENT(cmdList, name, color) \
        rhi::GPUScopedEvent CONCAT(_gpuEvent_, __LINE__)(cmdList, name, color)
#else
    #define GPU_SCOPED_EVENT(cmdList, name, color)
#endif

namespace rhi
{
    class GPUScopedEvent
    {
    public:
        GPUScopedEvent(CommandList* commandList, const char* name, std::uint8_t color = 0);
        ~GPUScopedEvent();

    private:
        CommandList* _commandList;
    };
} // namespace rhi
