#pragma once

#include <memory>

struct ID3D12Device2;

namespace tracking
{
    class ICommandListCrashContext;

    // Interface for GPU crash tracking implementations.
    class IGPUCrashTracker
    {
    public:
        // Virtual destructor.
        virtual ~IGPUCrashTracker() = default;

        // Enables GPU crash tracking.
        // Must be called before the device creation.
        virtual void Enable() = 0;

        // Initializes crash tracking with the given GPU Device.
        virtual void Initialize(ID3D12Device2* device) = 0;

        // Waits until any ongoing crash dump operation is finished.
        virtual void WaitUntilCrashDumpFinished() = 0;

        // Creates a crash context for a command list.
        std::unique_ptr<ICommandListCrashContext> CreateCommandListCrashContext();

        // Factory method to create a GPU crash tracker instance.
        static std::unique_ptr<IGPUCrashTracker> Create();
    };
} // namespace tracking
