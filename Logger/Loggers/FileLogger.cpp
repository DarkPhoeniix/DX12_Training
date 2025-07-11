
#include "FileLogger.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace logging
{
    FileLogger::FileLogger(const std::string& filePath)
    {
        // Create a file sink for logging
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        // [hours:minutes:seconds] [log_level] (thread_id) message
        sink->set_pattern("[%H:%M:%S] [%l] (%t) %v");

        _logger = std::make_unique<spdlog::logger>("FileLogger", std::move(sink));
    }

    void FileLogger::LogCritical(const char* message)
    {
        _logger->critical(message);
    }

    void FileLogger::LogError(const char* message)
    {
        _logger->error(message);
    }

    void FileLogger::LogWarning(const char* message)
    {
        _logger->warn(message);
    }

    void FileLogger::LogInfo(const char* message)
    {
        _logger->info(message);
    }

    void FileLogger::LogDebug(const char* message)
    {
        _logger->debug(message);
    }
} // namespace logging
