#pragma once

#include <string>
#include <memory>

namespace rhi
{
    class CommandList;
}

namespace tracking
{
    class IGPUCrashTracker;

    // Interface for command list crash context, which provides methods to set markers and 
    // initialize the command list for GPU crash tracking.
    class ICommandListCrashContext
    {
    public:
        // Virtual destructor.
        virtual ~ICommandListCrashContext() = default;

        // Initializes the command list crash context.
        virtual void Initialize(rhi::CommandList* commandList) = 0;
        // Sets a marker for the command list crash context.
        virtual void SetMarker(const char* marker) = 0;
        // Converts a string marker to a char pointer and calls SetMarker(const char*).
        virtual void SetMarker(const std::string& marker) final;
    };
} // namespace tracking
