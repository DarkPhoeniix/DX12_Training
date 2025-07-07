#pragma once

#include "../ILogger.h"

#include <spdlog/logger.h>

class ConsoleLogger : public ILogger
{
public:
    ~ConsoleLogger() override = default;

    void LogError(const char* message) override;
    void LogWarning(const char* message) override;
    void LogInfo(const char* message) override;
    void LogDebug(const char* message) override;
    void LogCritical(const char* message) override;

protected:
    std::shared_ptr<spdlog::logger> _logger;
};

