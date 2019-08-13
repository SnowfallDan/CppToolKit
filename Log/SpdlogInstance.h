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
    LOG2FILE = 0,
    LOG2STDOUT,
    BOTH,
};

class SpdlogInstance
{
public:
    static SpdlogInstance *get_instance(const string &log_file_path, const uint &log_level, const uint &log_output);

    std::shared_ptr<spdlog::logger> get_logger();

private:
    SpdlogInstance(const string &log_file_path, const uint &log_level, const uint &log_output);
    SpdlogInstance() = delete;

    SpdlogInstance(const SpdlogInstance &) = delete;
    SpdlogInstance &operator=(const SpdlogInstance &) = delete;

    std::shared_ptr<spdlog::logger> logger_;
    static SpdlogInstance *m_instance_;
};

#endif
