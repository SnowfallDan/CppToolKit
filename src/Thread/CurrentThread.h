#ifndef CPPTOOLKITS_CURRENTTHREAD_H
#define CPPTOOLKITS_CURRENTTHREAD_H

#include <sys/syscall.h>
#include <unistd.h>

namespace CurrentThread
{
    extern __thread int current_tid;

    inline int gettid()
    {
        return static_cast<int>(::syscall(SYS_gettid));
    }

    inline void cache_tid()
    {
        if (current_tid == 0)
            current_tid = gettid();
    }

    inline int which_tid()
    {
        if (__builtin_expect(current_tid == 0, 0))
            cache_tid();
        return current_tid;
    }
}

#endif //CPPTOOLKITS_CURRENTTHREAD_H
