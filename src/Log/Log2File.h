#ifndef ILOG2OUTPUT_H
#define LOG2OUTPUT_H

#ifdef LOG2OUTPUT
#include "LogPrinter.h"
#else
#include "SpdlogInstance.h"
extern SpdlogInstance *spd_log;
#endif

inline void log_debug(const string &module_name, const string &func_name, const string &msg)
{
#ifdef LOG2OUTPUT
    PRINT_DEBUG(module_name, func_name, msg);
#else
    if(spd_log->get_logger() != nullptr)
    {
        if(module_name.empty() && func_name.empty())
            spd_log->get_logger()->debug("[{}] {}", "DEBUG", msg);
        else
            spd_log->get_logger()->debug("[{}]<{}::{}> {}", "DEBUG", module_name, func_name, msg);
    }
#endif
}

inline void log_error(const string &module_name, const string &func_name, const string &msg)
{
#ifdef LOG2OUTPUT
    PRINT_ERROR(module_name, func_name, msg);
#else
    if(spd_log->get_logger() != nullptr)
    {
        if(module_name.empty() && func_name.empty())
            spd_log->get_logger()->error("[{}] {}", "ERROR", msg);
        else
            spd_log->get_logger()->error("[{}]<{}::{}> {}", "ERROR", module_name, func_name, msg);
    }
#endif
}

inline void log_info(const string &module_name, const string &func_name, const string &msg)
{
#ifdef LOG2OUTPUT
    PRINT_INFO(module_name, func_name, msg);
#else
    if(spd_log->get_logger() != nullptr)
    {
        if(module_name.empty() && func_name.empty())
            spd_log->get_logger()->info("[{}] {}", "INFO ", msg);
        else
            spd_log->get_logger()->info("[{}]<{}::{}> {}", "INFO ", module_name, func_name, msg);
    }
#endif
}

inline void log_warn(const string &module_name, const string &func_name, const string &msg)
{
#ifdef LOG2OUTPUT
    PRINT_INFO(module_name, func_name, msg);
#else
    if(spd_log->get_logger() != nullptr)
    {
        if(module_name.empty() && func_name.empty())
            spd_log->get_logger()->warn("[{}] {}", "WARN ", msg);
        else
            spd_log->get_logger()->warn("[{}]<{}::{}> {}", "WARN ", module_name, func_name, msg);
    }
#endif
}

inline void log_trace(const string &module_name, const string &func_name, const string &msg)
{
#ifdef LOG2OUTPUT
    PRINT_DEBUG(module_name, func_name, msg);
#else
    if(spd_log->get_logger() != nullptr)
    {
        if(module_name.empty() && func_name.empty())
            spd_log->get_logger()->trace("[{}] {}", "TRACE", msg);
        else
            spd_log->get_logger()->trace("[{}]<{}::{}> {}", "TRACE", module_name, func_name, msg);
    }

#endif
}

inline void log_msg(const string &in_or_out, const string &op, const string &id, const string &msg)
{
#ifdef LOG2OUTPUT
#else
    if(spd_log->get_logger() != nullptr)
        spd_log->get_logger()->info ("[{}]<{}::{}> {}", in_or_out, op, id, msg);
#endif
}

#define LOG_DEBUG(module_name, func_name, msg) log_debug(module_name, func_name, msg)
#define LOG_ERROR(module_name, func_name, msg) log_error(module_name, func_name, msg)
#define LOG_INFO(module_name, func_name, msg) log_info(module_name, func_name, msg)
#define LOG_WARN(module_name, func_name, msg) log_warn(module_name, func_name, msg)
#define LOG_TRACE(module_name, func_name, msg) log_trace(module_name, func_name, msg)

#define STRING(x) std::to_string(x)
#define LOG_INPUT(op, id, msg) log_msg("IN  ", op, id, msg)
#define LOG_OUTPUT(op, id, msg) log_msg("OUT ", op, id, msg)
#define LOG_DONE(op, id, msg) log_msg("DONE", op, id, msg)

#endif
