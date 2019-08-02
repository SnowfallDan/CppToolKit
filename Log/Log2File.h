#ifndef ICEBERG_LOG2OUTPUT_H
#define ICEBERG_LOG2OUTPUT_H

#ifdef LOG2OUTPUT
#include "LogPrinter.h"
#else
#include "SpdlogInstance.h"
extern SpdlogInstance *spd_log;
#endif

inline void log_debug(const string &module_name, const string &func_name, const string &msg)
{
    if(spd_log->get_logger() != nullptr)
        spd_log->get_logger()->debug("[{}]<{}::{}> {}", "DEBUG", module_name, func_name, msg);
}

inline void log_error(const string &module_name, const string &func_name, const string &msg)
{
    if(spd_log->get_logger() != nullptr)
        spd_log->get_logger()->error("[{}]<{}::{}> {}", "ERROR", module_name, func_name, msg);
}

inline void log_info(const string &module_name, const string &func_name, const string &msg)
{
    if(spd_log->get_logger() != nullptr)
        spd_log->get_logger()->info("[{}]<{}::{}> {}", "INFO ", module_name, func_name, msg);
}


#define LOG_DEBUG(module_name, func_name, msg) log_debug(module_name, func_name, msg)
#define LOG_ERROR(module_name, func_name, msg) log_error(module_name, func_name, msg)
#define LOG_INFO(module_name, func_name, msg) log_info(module_name, func_name, msg)

#endif
