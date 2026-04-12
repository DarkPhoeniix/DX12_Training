#pragma once

#include "CommandList.h"

#if ENABLE_GPU_EVENTS
    #define CONCAT_IMPL(x, y) x##y
    #define CONCAT(x, y) CONCAT_IMPL(x, y)
    #define GPU_SCOPED_EVENT(cmdList, name, color) \
        rhi::GPUScopedEvent CONCAT(_gpuEvent_, __LINE__)(cmdList, name, color)
#else
    #define GPU_SCOPED_EVENT(cmdList, name, color)
#endif // ENABLE_GPU_EVENTS

namespace rhi
{
    // GPUScopedEvent is a utility class that manages GPU events in a scoped manner. 
    // It allows the application to mark specific sections of GPU work with named events. 
    // It automatically starts and ends the GPU event, ensuring proper pairing of begin and end calls for GPU events.
    class GPUScopedEvent
    {
    public:
        // Constructs a GPUScopedEvent object, starting a GPU event with the specified name and color
        GPUScopedEvent(CommandList* commandList, const char* name, std::uint8_t color = 0);
        // Destructs the GPUScopedEvent object, ending the GPU event that was started in the constructor
        ~GPUScopedEvent();

    protected:
        CommandList* _commandList;
    };
} // namespace rhi
