#include <iostream>
#include "Log/Log2File.h"

SpdlogInstance *spd_log = nullptr;

int main()
{
    string log_path = "D:\\test.log";

    spd_log = SpdlogInstance::get_instance(log_path,
                                           LOG_DEBUG,
                                           BOTH);

    LOG_INFO("TEST", "TEST", "TEST");

    return 0;
}