#pragma once

#include <memory>
#include <format>

#ifdef _DEBUG
#define FAIL(statement, message) \
    do { \
        if (!(statement)) \
        { \
            logging::Logger::Instance().Log(logging::Level::Critical, "[%s (%u)] %s", __func__, __LINE__, message); \
            __debugbreak(); \
        } \
    } while (0)
#define ASSERT(statement, message) \
    do { \
        if (!(statement)) \
        { \
            logging::Logger::Instance().Log(logging::Level::Error, "[%s (%u)] %s", __func__, __LINE__, message); \
            __debugbreak(); \
        } \
    } while (0)
#define CHECK(hResult, message) \
    do { \
        if (FAILED(hResult)) \
        { \
            logging::Logger::Instance().Log(logging::Level::Error, "[%s (%u)] %s \nError code: %u", __func__, __LINE__, hResult); \
            __debugbreak(); \
        } \
    } while (0)
#define UNREACHABLE(message) \
    do { \
        logging::Logger::Instance().Log(logging::Level::Critical, "[%s (%u)] %s", __func__, __LINE__, message); \
        __debugbreak(); \
    } while (0)
#define NOT_IMPLEMENTED() \
    do { \
        logging::Logger::Instance().Log(logging::Level::Critical, "[%s (%u)] NOT IMPLEMENTED!", __func__, __LINE__); \
        __debugbreak(); \
    } while (0)
#else
#define FAIL(...)
#define ASSERT(...)
#define CHECK(...)
#define UNREACHABLE(...)
#define NOT_IMPLEMENTED()
#endif // _DEBUG

#define LOG_DEBUG(message, ...) \
    logging::Logger::Instance().Log(logging::Level::Debug, message, __VA_ARGS__)
#define LOG_INFO(message, ...) \
    logging::Logger::Instance().Log(logging::Level::Info, message, __VA_ARGS__)
#define LOG_WARNING(message, ...) \
    logging::Logger::Instance().Log(logging::Level::Warning, message, __VA_ARGS__)
#define LOG_ERROR(message, ...) \
    logging::Logger::Instance().Log(logging::Level::Error, message, __VA_ARGS__)
#define LOG_CRITICAL(message, ...) \
    logging::Logger::Instance().Log(logging::Level::Critical, message, __VA_ARGS__)

namespace logging
{
    enum class Level
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

        static void Init(const std::string& logFilePath = "log.txt");
        static void Shutdown();

        static Logger& Instance();

        template<typename... Args>
        void Log(Level level, const std::string& message, Args&& ...args);
        void Log(Level level, const std::string& message);

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

    template<typename ...Args>
    void Logger::Log(Level level, const std::string& message, Args&& ...args)
    {
        // TODO: check if this conversion will not cause performance issues
        // It'll be better to avoid string conversion if possible
        std::string str = std::vformat(message.c_str(), std::make_format_args(args...));
        Log(level, str);
    }

} // namespace logging
