#ifndef MYUTILS_TIME_H
#define MYUTILS_TIME_H

#include <unistd.h>
#include <thread>
#include <sys/time.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <mutex>
#include <string>
#include <algorithm>
#include <functional>
#include <type_traits>

using std::atomic;

static inline uint64_t getCurrentMicrosecondOrigin() {
#if !defined(_WIN32)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
#else
    return  std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#endif
}
static atomic<uint64_t> s_currentMicrosecond(0);
static atomic<uint64_t> s_currentMillisecond(0);
static atomic<uint64_t> s_currentMicrosecond_system(getCurrentMicrosecondOrigin());
static atomic<uint64_t> s_currentMillisecond_system(getCurrentMicrosecondOrigin() / 1000);

static inline bool initMillisecondThread() {
    static std::thread s_thread([]() {
        uint64_t last = getCurrentMicrosecondOrigin();
        uint64_t now;
        uint64_t microsecond = 0;
        while (true) {
            now = getCurrentMicrosecondOrigin();
            //记录系统时间戳，可回退
            s_currentMicrosecond_system.store(now, std::memory_order_release);
            s_currentMillisecond_system.store(now / 1000, std::memory_order_release);

            //记录流逝时间戳，不可回退
            int64_t expired = now - last;
            last = now;
            if (expired > 0 && expired < 1000 * 1000) {
                //流逝时间处于0~1000ms之间，那么是合理的，说明没有调整系统时间
                microsecond += expired;
                s_currentMicrosecond.store(microsecond, std::memory_order_release);
                s_currentMillisecond.store(microsecond / 1000, std::memory_order_release);
            } else if(expired != 0){
                std::cout << "Stamp expired is not abnormal:" << expired << std::endl;
            }

            //休眠0.5 ms
            usleep(500);
        }
    });

    s_thread.detach();
    return true;
}

uint64_t getCurrentMillisecond(bool system_time = false)
{
    static bool flag = initMillisecondThread();
    if (system_time)
    {
        return s_currentMillisecond_system.load(std::memory_order_acquire);
    }
    return s_currentMillisecond.load(std::memory_order_acquire);
}

uint64_t getCurrentMicrosecond(bool system_time = false)
{
    static bool flag = initMillisecondThread();
    if (system_time)
    {
        return s_currentMicrosecond_system.load(std::memory_order_acquire);
    }
    return s_currentMicrosecond.load(std::memory_order_acquire);
}

string getTimeStr(const char *fmt, time_t time)
{
    std::tm tm_snapshot;
    if (!time)
    {
        time = ::time(NULL);
    }

    localtime_r(&time, &tm_snapshot); // POSIX
    char buffer[1024];
    auto success = std::strftime(buffer, sizeof(buffer), fmt, &tm_snapshot);
    if (0 == success)
        return string(fmt);
    return buffer;
}

#endif //MYUTILS_TIME_H
