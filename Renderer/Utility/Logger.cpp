#include "RendererPCH.h"

#include "Logger.h"

namespace
{
    const std::string LOG_FILEPATH = "log.txt";

    const std::map<LogType, std::string> LOG_TYPE =
    {
        { LogType::Info, "Info" },
        { LogType::Warning, "Warning" },
        { LogType::Error, "Error" }
    };

    std::string logType(LogType type)
    {
        auto it = LOG_TYPE.find(type);
        if (it != LOG_TYPE.end())
        {
            return it->second;
        }
        return LOG_TYPE.begin()->second;
    }
}

namespace assert_utility
{
    bool AssertFunction(bool statement, const std::string& message)
    {
        return AssertFunction(statement, message.c_str());
    }

    bool AssertFunction(bool statement, const char* message)
    {
        if (!statement)
        {
            Logger::Log(LogType::Error, message);
#if defined(_DEBUG)
            OutputDebugStringA(message);
            OutputDebugStringA("\n");
#endif
        }
        return !statement;
    }

    bool LogWarningFunction(bool statement, const std::string& message)
    {
        return LogWarningFunction(statement, message.c_str());
    }

    bool LogWarningFunction(bool statement, const char* message)
    {
        if (!statement)
        {
            Logger::Log(LogType::Warning, message);
#if defined(_DEBUG)
            OutputDebugStringA(message);
            OutputDebugStringA("\n");
#endif
        }
        return !statement;
    }

    void LogInfoFunction(const std::string& message)
    {
        LogInfoFunction(message.c_str());
    }

    void LogInfoFunction(const char* message)
    {
#if defined(_DEBUG)
        Logger::Log(LogType::Info, message);
#endif
    }
}

#if _DEBUG
LogType Logger::_logLevel = LogType::Error;
#elif NDEBUG
LogType Logger::_logLevel = LogType::Error;
#endif

Logger& Logger::Instance()
{
    static Logger logger;
    return logger;
}

void Logger::Log(LogType type, const std::string& message)
{
    Log(type, message.c_str());
}

void Logger::Log(LogType type, const char* message)
{
    std::chrono::time_point t = std::chrono::system_clock::now();
    auto time = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_local_time();

    Instance()._logFile << std::format("{0:%T}", time) << " | " << logType(type) << ": " << message << std::endl;
}

void Logger::SetLogLevel(LogType logLevel)
{
    _logLevel = logLevel;
}

LogType Logger::GetLogLevel()
{
    return _logLevel;
}

Logger::Logger()
{
    _logFile.open(LOG_FILEPATH, std::ios_base::out);
}

Logger::~Logger()
{
    _logFile.close();
}
