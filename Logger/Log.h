#pragma once

#include <string>
#include <memory>
#include <vector>

namespace logging
{
    class ILogger;
    enum class LogLevel;

    class Logger final
    {
    public:
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        ~Logger() = default;

        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&) = default;

        static void Init();
        static void Shutdown();

        static void Log(LogLevel level, const char* message);

        static void Critical(const char* message);
        static void Critical(const std::string& message);

        static void Error(const char* message);
        static void Error(const std::string& message);

        static void Warning(const char* message);
        static void Warning(const std::string& message);

        static void Info(const char* message);
        static void Info(const std::string& message);

        static void Debug(const char* message);
        static void Debug(const std::string& message);

    private:
        Logger();

        static void InitLoggers();

        std::vector<std::unique_ptr<ILogger>> _loggers;

        static std::unique_ptr<Logger> _instance;
};
} // namespace logging