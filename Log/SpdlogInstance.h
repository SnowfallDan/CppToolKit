#ifndef ICEBERG_SPDLOGINSTANCE_H
#define ICEBERG_SPDLOGINSTANCE_H

#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#if defined(__linux__)
// Linux
#elif defined(_WIN32)
#define uint uint32_t
#endif

using std::string;

enum LOG_LEVEL
{
    LOG_TRACE = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL,
    LOG_OFF,
};

enum LOG_OUTPUT
{
    LOG2STDOUT = 0,
    LOG2FILE,
    BOTH,
};

spdlog::level::level_enum calculate_level(const uint &log_level)
{
    switch (log_level)
    {
        case LOG_TRACE:
            return spdlog::level::trace;
        case LOG_DEBUG:
            return spdlog::level::debug;
        case LOG_INFO:
            return spdlog::level::info;
        case LOG_WARN:
            return spdlog::level::warn;
        case LOG_ERROR:
            return spdlog::level::err;
        case LOG_CRITICAL:
            return spdlog::level::critical;
        case LOG_OFF:
            return spdlog::level::off;
        default:
            return spdlog::level::debug;
    }
}

class SpdlogInstance
{
public:
    static SpdlogInstance *get_instance(const string &log_file_path, const uint &log_level, const uint &log_output)
    {
        if(m_instance_ == nullptr)
            m_instance_ = new SpdlogInstance(log_file_path, log_level, log_output);

        return m_instance_;
    }

    std::shared_ptr<spdlog::logger> get_logger()
    {
        return logger_;
    }

private:
    SpdlogInstance(const string &log_file_path, const uint &log_level, const uint &log_output)
    {
        std::vector<spdlog::sink_ptr> sink_list;
        // 标准输出
        if(log_output == 0 || log_output == 2)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(calculate_level(log_level));
            sink_list.push_back(console_sink);
        }

        // 文件输出
        if(log_output == 1 || log_output == 2)
        {
            auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(log_file_path, 0, 1);
            file_sink->set_level(calculate_level(log_level));
            sink_list.push_back(file_sink);
        }

        if(sink_list.empty())
            return;

        logger_ = std::make_shared<spdlog::logger>("both", begin(sink_list), end(sink_list));
        spdlog::register_logger(logger_);

        logger_->set_level(calculate_level(log_level));

        logger_->set_pattern("%^[%Y-%m-%d %H:%M:%S %f][Thread: %t] %v %$");
        logger_->flush_on(calculate_level(log_level));
    }

    SpdlogInstance() = delete;

    SpdlogInstance(const SpdlogInstance &) = delete;
    SpdlogInstance &operator=(const SpdlogInstance &) = delete;

    std::shared_ptr<spdlog::logger> logger_;
    static SpdlogInstance *m_instance_;
};

SpdlogInstance *SpdlogInstance::m_instance_ = nullptr;

#endif
