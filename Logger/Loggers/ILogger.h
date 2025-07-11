#pragma once

namespace logging
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        virtual void Log(LogLevel level, const char* message) final;

        virtual void LogDebug(const char* message) = 0;
        virtual void LogInfo(const char* message) = 0;
        virtual void LogWarning(const char* message) = 0;
        virtual void LogError(const char* message) = 0;
        virtual void LogCritical(const char* message) = 0;
    };
} // namespace logging
