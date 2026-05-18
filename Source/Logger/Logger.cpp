
#include "Logger.h"

#include <spdlog/spdlog.h>
#ifdef MSVC_LOG
#include <spdlog/sinks/msvc_sink.h>
#endif // MSVC_LOG
#ifdef FILE_LOG
#include <spdlog/sinks/basic_file_sink.h>
#endif // FILE_LOG

namespace logging 
{
    namespace
    {
        // Define a log pattern that includes timestamp, log level, thread id, and the log message
        constexpr const char* LOG_PATTERN = "[%H:%M:%S] [%l] (%t) %v";
    } // namespace unnamed

    struct Logger::Impl
    {
        std::shared_ptr<spdlog::logger> SpdLogger;
    };

    void Logger::Init(const std::string& logFilePath)
    {
        // Create sinks
        std::vector<spdlog::sink_ptr> sinks;
#ifdef MSVC_LOG
        auto msvc = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        msvc->set_pattern(LOG_PATTERN);
        sinks.push_back(msvc);
#endif // MSVC_LOG

#ifdef FILE_LOG
        auto file = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
        file->set_pattern(LOG_PATTERN);
        sinks.push_back(file);
#endif // FILE_LOG

        // Construct the logger
        Instance()._impl->SpdLogger = std::make_shared<spdlog::logger>("EngineLogger", std::begin(sinks), std::end(sinks));
        Instance()._impl->SpdLogger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(Instance()._impl->SpdLogger);
    }

    void Logger::Shutdown()
    {
        // Flush and shutdown the logger
        spdlog::shutdown();
        // Reset the default logger to nullptr
        spdlog::set_default_logger(nullptr);
    }

    Logger& Logger::Instance()
    {
        static Logger instance;
        return instance;
    }

    void Logger::Log(Level level, const std::string& message)
    {
        switch (level)
        {
        case Level::Debug:
            _impl->SpdLogger->debug(message);
            break;
        case Level::Info:
            _impl->SpdLogger->info(message);
            break;
        case Level::Warning:
            _impl->SpdLogger->warn(message);
            break;
        case Level::Error:
            _impl->SpdLogger->error(message);
            break;
        case Level::Critical:
            _impl->SpdLogger->critical(message);
            break;
        }
    }

    void Logger::Debug(const std::string& msg)
    {
        Log(Level::Debug, msg);
    }

    void Logger::Info(const std::string& msg)
    {
        Log(Level::Info, msg);
    }

    void Logger::Warning(const std::string& msg)
    {
        Log(Level::Warning, msg);
    }

    void Logger::Error(const std::string& msg)
    {
        Log(Level::Error, msg);
    }

    void Logger::Critical(const std::string& msg)
    {
        Log(Level::Critical, msg);
    }

    Logger::Logger()
        : _impl(std::make_unique<Impl>())
    {
        _impl->SpdLogger = spdlog::default_logger();
    }
} // namespace logging
