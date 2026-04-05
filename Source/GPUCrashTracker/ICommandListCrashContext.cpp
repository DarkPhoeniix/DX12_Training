
#include "ICommandListCrashContext.h"

namespace tracking
{
    void ICommandListCrashContext::SetMarker(const std::string& marker)
    {
        SetMarker(marker.c_str());
    }
} // namespace tracking
