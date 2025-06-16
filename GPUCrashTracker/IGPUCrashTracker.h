#pragma once

#include <memory>
#include <array>
#include <string>
#include <map>

struct ID3D12Device2;

namespace tracking
{
    class ICommandListCrashContext;

    // Interface for GPU crash tracking implementations.
    class IGPUCrashTracker : public std::enable_shared_from_this<IGPUCrashTracker>
    {
    public:
        // keep four frames worth of marker history
        const static std::uint32_t MarkerFrameHistory = 4;
        using MarkerMap = std::array<std::map<uint64_t, std::string>, MarkerFrameHistory>;

        // Default constructor.
        IGPUCrashTracker() = default;
        // Virtual destructor.
        virtual ~IGPUCrashTracker() = default;

        // Enables GPU crash tracking.
        // Must be called before the device creation.
        virtual void Enable() = 0;

        // Initializes crash tracking with the given GPU Device.
        virtual void Initialize(ID3D12Device2* device) = 0;

        // Waits until any ongoing crash dump operation is finished.
        virtual void WaitUntilCrashDumpFinished() = 0;

        // Advances the frame index for correct marker tracking.
        void AdvanceFrame();
        // Returns the current marker frame index.
        std::uint16_t GetMarkerFrameIndex() const;

        // Returns the marker map for app-managed markers.
        MarkerMap& GetMarkerMap();
        // Resets the marker map for the current frame index.
        void ResetMarkerMapForCurrentFrame();

        // Creates a crash context for a command list.
        std::shared_ptr<ICommandListCrashContext> CreateCommandListCrashContext();

        // Factory method to create a GPU crash tracker instance.
        static std::shared_ptr<IGPUCrashTracker> Create();

    protected:
        // App-managed marker tracking
        MarkerMap m_markerMap;
        // Current frame index for marker tracking.
        std::uint16_t m_markerFrameIndex;
    };
} // namespace tracking
