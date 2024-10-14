#pragma once

#define ASSERT(statement, message) \
    AssertUtility::AssertFunction(statement, message)

#define LOG_WARNING(statement, message) \
    AssertUtility::LogWarningFunction(statement, message)

#define LOG_INFO(message) \
    AssertUtility::LogInfoFunction(message)

namespace AssertUtility
{
    bool AssertFunction(bool statement, const std::string& message);
    bool LogWarningFunction(bool statement, const std::string& message);
    void LogInfoFunction(const std::string& message);
}

enum class LogType
{
    Info    = 1,
    Warning = 2,
    Error   = 4
};
BINARY_OPERATION_TO_ENUM(LogType);

class Logger
{
public:
    Logger(const Logger& copy) = delete;
    Logger& operator=(const Logger& copy) = delete;

    static Logger& Instance();

    static void Log(LogType type, const std::string& message);
    static void SetLogLevel(LogType logLevel);
    static LogType GetLogLevel();

private:
    Logger();
    ~Logger();

    std::fstream _logFile;

    static LogType _logLevel;
};
