#include <iostream>
#include "Log/Log2File.h"

SpdlogInstance *spd_log = nullptr;

int main()
{
    string log_path = "/home/snowfall/bin/test.log";

    spd_log = SpdlogInstance::get_instance(log_path,
                                           LOG_DEBUG,
                                           BOTH);

    for (int i = 0; i < 10; ++i)
    {
        LOG_INFO("TEST", "TEST", "TEST");
        sleep(1);
    }

    return 0;
}