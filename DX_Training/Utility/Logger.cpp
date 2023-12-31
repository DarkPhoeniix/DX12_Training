#include "stdafx.h"

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

LogType operator&(LogType lhs, LogType rhs)
{
    return static_cast<LogType>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

LogType operator|(LogType lhs, LogType rhs)
{
    return static_cast<LogType>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

#if _DEBUG
LogType Logger::_logLevel = LogType::Info | LogType::Warning | LogType::Error;
#elif NDEBUG
LogType Logger::_logLevel = LogType::Error;
#endif

void Logger::Log(LogType type, const std::string& message)
{
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now()).get_sys_time();

    std::ofstream file(LOG_FILEPATH);
    
    file << std::format("{0:%T}", time) << " | " << logType(type) << ": " << message << std::endl;
}

void Logger::SetLogLevel(LogType logLevel)
{
    _logLevel = logLevel;
}

LogType Logger::GetLogLevel()
{
    return _logLevel;
}
