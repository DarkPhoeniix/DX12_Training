#pragma once

#include "ILogger.h"

#include <spdlog/logger.h>

namespace logging
{
    class ConsoleLogger : public ILogger
    {
    public:
        ConsoleLogger();
        ConsoleLogger(const ConsoleLogger&) = delete;
        ConsoleLogger(ConsoleLogger&&) = default;
        ~ConsoleLogger() override = default;

        ConsoleLogger& operator=(const ConsoleLogger&) = delete;
        ConsoleLogger& operator=(ConsoleLogger&&) = default;

        void LogCritical(const char* message) override;
        void LogError(const char* message) override;
        void LogWarning(const char* message) override;
        void LogInfo(const char* message) override;
        void LogDebug(const char* message) override;

    protected:
        std::unique_ptr<spdlog::logger> _logger;
    };
} // namespace logging
