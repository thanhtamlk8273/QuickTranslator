#ifndef LOG_H
#define LOG_H

#include <vector>
#include <string>
#include <fmt/core.h>
#include <iostream>

namespace detail {
    enum class Level {
        LOG_INFO = 0,
        LOG_ERROR
    };

    inline const char* expand(Level level)
    {
        switch (level)
        {
            case Level::LOG_INFO: return "[INFO]";
            case Level::LOG_ERROR: return "[ERROR]";
            default: return "[UNKNOWN]";
        }
    }
}

class Log
{
private:
    std::string m_log_id;
public:
    using Level = detail::Level;
    /* Constructors */
    Log() = default;
    Log(const std::string& log_id): m_log_id(log_id) {};
    /* Normal methods */
    template<class... T>
    void log(Log::Level level, const std::string& fmt, T... args);
};

template<class... T>
inline void Log::log(Log::Level level, const std::string& fmt, T... args)
{
    std::string log_line = fmt::format(fmt + "\n", args...);
    std::cout << detail::expand(level) << '[' <<m_log_id <<']' << log_line;
}
#endif // LOG_H
