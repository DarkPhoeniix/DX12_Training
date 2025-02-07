#pragma once

#include <fstream>

#define ASSERT(statement, message) \
    assert(statement); \
    assert_utility::AssertFunction(statement, message)

#define LOG_WARNING(statement, message) \
    assert_utility::LogWarningFunction(statement, message)

#define LOG_INFO(message) \
    assert_utility::LogInfoFunction(message)

namespace assert_utility
{
    bool AssertFunction(bool statement, const std::string& message);
    bool AssertFunction(bool statement, const char* message);
    bool LogWarningFunction(bool statement, const std::string& message);
    bool LogWarningFunction(bool statement, const char* message);
    void LogInfoFunction(const std::string& message);
    void LogInfoFunction(const char* message);
} // namespace assert_utility

enum class LogType
{
    Info    = 1,
    Warning = 2,
    Error   = 4
};
//BINARY_OPERATION_TO_ENUM(LogType);

class Logger
{
public:
    Logger(const Logger& copy) = delete;
    Logger& operator=(const Logger& copy) = delete;

    static Logger& Instance();

    static void Log(LogType type, const std::string& message);
    static void Log(LogType type, const char* message);
    static void SetLogLevel(LogType logLevel);
    static LogType GetLogLevel();

private:
    Logger();
    ~Logger();

    std::fstream _logFile;

    static LogType _logLevel;
};
