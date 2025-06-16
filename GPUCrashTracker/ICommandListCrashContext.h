#pragma once

#include <string>
#include <memory>

struct ID3D12GraphicsCommandList;

namespace tracking
{
    class IGPUCrashTracker;

    // Interface for command list crash context, which provides methods to set markers and 
    // initialize the command list for GPU crash tracking.
    class ICommandListCrashContext
    {
    public:
        // Constructor with reference to the marker map from the crash tracker.
        ICommandListCrashContext(std::shared_ptr<IGPUCrashTracker> crashTracker);
        // Virtual destructor.
        virtual ~ICommandListCrashContext() = default;

        // Initializes the command list crash context.
        virtual void Initialize(ID3D12GraphicsCommandList* commandList) = 0;
        // Sets a marker for the command list crash context.
        virtual void SetMarker(const char* marker) = 0;
        // Converts a string marker to a char pointer and calls SetMarker(const char*).
        virtual void SetMarker(const std::string& marker) final;

    protected:
        std::shared_ptr<IGPUCrashTracker> _crashTracker;
    };
} // namespace tracking
