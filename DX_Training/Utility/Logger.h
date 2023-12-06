#pragma once

enum class LogType
{
    Info    = 1,
    Warning = 2,
    Error   = 4
};

LogType operator&(LogType lhs, LogType rhs);
LogType operator|(LogType lhs, LogType rhs);

class Logger
{
public:
    static void Log(LogType type, const std::string& message);
    static void SetLogLevel(LogType logLevel);
    static LogType GetLogLevel();

private:
    static LogType _logLevel;
};
