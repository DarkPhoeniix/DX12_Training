
#include "IGPUCrashTracker.h"

#ifdef USE_NSIGHT_AFTERMATH
#include "D3D12CrashTracker/NsightAftermathCommandListContext.h"
#include "D3D12CrashTracker/NsightAftermathGpuCrashTracker.h"
#else
#include "NullCrashTracker/NullCommandListCrashContext.h"
#include "NullCrashTracker/NullCrashTracker.h"
#endif

namespace tracking
{
    std::unique_ptr<ICommandListCrashContext> IGPUCrashTracker::CreateCommandListCrashContext()
    {
#ifdef USE_NSIGHT_AFTERMATH
        return std::make_unique<NsightAftermathCommandListContext>();
#else
        return std::make_unique<NullCommandListCrashContext>();
#endif
    }

    std::unique_ptr<IGPUCrashTracker> IGPUCrashTracker::Create()
    {
#ifdef USE_NSIGHT_AFTERMATH
        return std::make_unique<GpuCrashTracker>();
#else
        return std::make_unique<NullCrashTracker>();
#endif
    }
} // namespace tracking
