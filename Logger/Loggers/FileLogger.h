#pragma once

#include "ILogger.h"

#include <spdlog/logger.h>

namespace logging
{
    class FileLogger : public ILogger
    {
    public:
        FileLogger(const std::string& filePath);
        FileLogger(const FileLogger&) = delete;
        FileLogger(FileLogger&&) = default;
        ~FileLogger() override = default;

        FileLogger& operator=(const FileLogger&) = delete;
        FileLogger& operator=(FileLogger&&) = default;

        void LogCritical(const char* message) override;
        void LogError(const char* message) override;
        void LogWarning(const char* message) override;
        void LogInfo(const char* message) override;
        void LogDebug(const char* message) override;

    protected:
        std::unique_ptr<spdlog::logger> _logger;
    };
} // namespace logging
