#ifndef CPPTOOLKITS_MUTEXGUARD_H
#define CPPTOOLKITS_MUTEXGUARD_H

#include "Mutex.h"

class MutexLockGuard
{
public:
    explicit MutexLockGuard(Mutex &lock) : lock_(lock)
    {
        lock_.lock();
    }

    explicit MutexLockGuard(Mutex *lock) : lock_(*lock)
    {
        lock_.lock();
    }

    ~MutexLockGuard()
    {
        lock_.unlock();
    }

private:
    Mutex &lock_;
};

#endif //CPPTOOLKITS_MUTEXGUARD_H