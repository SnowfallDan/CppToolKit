#include <iostream>
#include "Log2File.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <SpdlogInstance.h>

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