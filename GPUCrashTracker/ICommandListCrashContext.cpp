
#include "ICommandListCrashContext.h"

namespace tracking
{
    ICommandListCrashContext::ICommandListCrashContext(std::shared_ptr<IGPUCrashTracker> crashTracker)
        : _crashTracker(crashTracker)
    {
    }

    void ICommandListCrashContext::SetMarker(const std::string& marker)
    {
        SetMarker(marker.c_str());
    }
} // namespace tracking
