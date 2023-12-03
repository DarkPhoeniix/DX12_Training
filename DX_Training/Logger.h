#pragma once

enum class LogType
{
    Info,
    Warning,
    Error
};

class Logger
{
public:
    static void Log(LogType type, const std::string& message);
};
