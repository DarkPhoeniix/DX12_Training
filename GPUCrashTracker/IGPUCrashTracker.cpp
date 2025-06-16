
#include "IGPUCrashTracker.h"

#ifdef USE_NSIGHT_AFTERMATH
#include "D3D12CrashTracker/NsightAftermathCommandListContext.h"
#include "D3D12CrashTracker/NsightAftermathGpuCrashTracker.h"
#else
#include "NullCrashTracker/NullCommandListCrashContext.h"
#include "NullCrashTracker/NullCrashTracker.h"
#endif

#include <cassert>

namespace tracking
{
    void IGPUCrashTracker::AdvanceFrame()
    {
        m_markerFrameIndex = (m_markerFrameIndex + 1) % IGPUCrashTracker::MarkerFrameHistory;
    }

    std::uint16_t IGPUCrashTracker::GetMarkerFrameIndex() const
    {
        return m_markerFrameIndex;
    }

    IGPUCrashTracker::MarkerMap& IGPUCrashTracker::GetMarkerMap()
    {
        return m_markerMap;
    }

    void IGPUCrashTracker::ResetMarkerMapForCurrentFrame()
    {
        m_markerMap[m_markerFrameIndex].clear();
    }

    std::shared_ptr<ICommandListCrashContext> IGPUCrashTracker::CreateCommandListCrashContext()
    {
#ifdef USE_NSIGHT_AFTERMATH
        return std::make_shared<NsightAftermathCommandListContext>(shared_from_this());
#else
        return std::make_shared<NullCommandListCrashContext>(shared_from_this());
#endif
    }

    std::shared_ptr<IGPUCrashTracker> IGPUCrashTracker::Create()
    {
#ifdef USE_NSIGHT_AFTERMATH
        return std::make_shared<GpuCrashTracker>();
#else
        return std::make_shared<NullCrashTracker>();
#endif
    }
} // namespace tracking
