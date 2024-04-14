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
