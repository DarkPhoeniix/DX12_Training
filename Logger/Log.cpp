
#include "Log.h"

#include "Loggers/ILogger.h"
#ifdef MSVC_LOG
#include "Loggers/ConsoleLogger.h"
#endif // MSVC_LOG
#ifdef FILE_LOG
#include "Loggers/FileLogger.h"
#endif // FILE_LOG

namespace logging
{
    namespace
    {
        static const char* LOG_FILEPATH = "log.txt";
    }

    std::unique_ptr<Logger> Logger::_instance = nullptr;

    Logger::Logger()
    {
        InitLoggers();
    }

    void Logger::InitLoggers()
    {
#ifdef MSVC_LOG
        _instance->_loggers.emplace_back(std::make_unique<ConsoleLogger>());
#endif // MSVC_LOG

#ifdef FILE_LOG
        _instance->_loggers.emplace_back(std::make_unique<FileLogger>(LOG_FILEPATH));
#endif // FILE_LOG
    }

    void Logger::Init()
    {
        if (!_instance)
        {
            _instance = std::unique_ptr<Logger>(new Logger);
        }
    }

    void Logger::Shutdown()
    {
        _instance.reset();
    }

    void Logger::Log(LogLevel level, const char* message)
    {
        for (const auto& logger : _instance->_loggers)
        {
            logger->Log(level, message);
        }
    }

    void Logger::Critical(const char* message)
    {
        Log(LogLevel::Critical, message);
    }

    void Logger::Critical(const std::string& message)
    {
        Log(LogLevel::Critical, message.c_str());
    }

    void Logger::Error(const char* message)
    {
        Log(LogLevel::Error, message);
    }

    void Logger::Error(const std::string& message)
    {
        Log(LogLevel::Error, message.c_str());
    }

    void Logger::Warning(const char* message)
    {
        Log(LogLevel::Warning, message);
    }

    void Logger::Warning(const std::string& message)
    {
        Log(LogLevel::Warning, message.c_str());
    }

    void Logger::Info(const char* message)
    {
        Log(LogLevel::Info, message);
    }

    void Logger::Info(const std::string& message)
    {
        Log(LogLevel::Info, message.c_str());
    }

    void Logger::Debug(const char* message)
    {
        Log(LogLevel::Debug, message);
    }

    void Logger::Debug(const std::string& message)
    {
        Log(LogLevel::Debug, message.c_str());
    }
} // namespace logging
