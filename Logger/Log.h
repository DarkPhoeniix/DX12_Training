#pragma once

#include <memory>
#include <string>

#define LOG_DEBUG(message) \
    logging::Logger::Instance().Log(logging::LogLevel::Debug, message)
#define LOG_INFO(message) \
    logging::Logger::Instance().Log(logging::LogLevel::Info, message)
#define LOG_WARNING(message) \
    logging::Logger::Instance().Log(logging::LogLevel::Warning, message)
#define LOG_ERROR(message) \
    logging::Logger::Instance().Log(logging::LogLevel::Error, message)
#define LOG_CRITICAL(message) \
    logging::Logger::Instance().Log(logging::LogLevel::Critical, message)

namespace logging
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger(Logger&&) = default;

        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&) = default;

        static void Init(const std::string& logFilePath = "engine.log");
        static void Shutdown();

        static Logger& Instance();

        void Log(LogLevel level, const std::string& message);

        void Debug(const std::string& msg);
        void Info(const std::string& msg);
        void Warning(const std::string& msg);
        void Error(const std::string& msg);
        void Critical(const std::string& msg);

    private:
        Logger();
        ~Logger() = default;

        // hide spdlog from public headers
        struct Impl;
        std::unique_ptr<Impl> _impl;
    };

} // namespace logging
