#include "ILogger.h"

namespace logging
{
    void ILogger::Log(LogLevel level, const char* message)
    {
        switch (level)
        {
        case LogLevel::Debug:
            LogDebug(message);
            break;
        case LogLevel::Info:
            LogInfo(message);
            break;
        case LogLevel::Warning:
            LogWarning(message);
            break;
        case LogLevel::Error:
            LogError(message);
            break;
        case LogLevel::Critical:
            LogCritical(message);
            break;
        }
    }
} // namespace logging
