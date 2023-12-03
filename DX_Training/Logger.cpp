#include "stdafx.h"

#include "Logger.h"

#include <fstream>
#include <iomanip>
#include <ctime>

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

void Logger::Log(LogType type, const std::string& message)
{
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm localTime = *std::localtime(&time);

    std::ofstream file(LOG_FILEPATH);

    file << std::put_time(&localTime, "%H:%M:%S") << " | " << logType(type) << ": " << message << std::endl;
}
