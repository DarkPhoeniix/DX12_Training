
#include "ConsoleLogger.h"

#include <spdlog/sinks/msvc_sink.h>

namespace logging
{
    ConsoleLogger::ConsoleLogger()
    {
        // Create a file sink for logging
        auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        // [hours:minutes:seconds] [log_level] (thread_id) message
        sink->set_pattern("[%H:%M:%S] [%l] (%t) %v");

        _logger = std::make_unique<spdlog::logger>("MSVCConsoleLogger", std::move(sink));
    }

    void ConsoleLogger::LogCritical(const char* message)
    {
        _logger->critical(message);
    }

    void ConsoleLogger::LogError(const char* message)
    {
        _logger->error(message);
    }

    void ConsoleLogger::LogWarning(const char* message)
    {
        _logger->warn(message);
    }

    void ConsoleLogger::LogInfo(const char* message)
    {
        _logger->info(message);
    }

    void ConsoleLogger::LogDebug(const char* message)
    {
        _logger->debug(message);
    }
} // namespace logging
