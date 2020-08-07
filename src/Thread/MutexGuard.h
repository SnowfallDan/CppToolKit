#ifndef CPPTOOLKITS_MUTEXGUARD_H
#define CPPTOOLKITS_MUTEXGUARD_H

#include "Mutex.h"

class MutexGuard
{
public:
    explicit MutexGuard(Mutex &lock) : lock_(lock)
    {
        lock_.lock();
    }

    explicit MutexGuard(Mutex *lock) : lock_(*lock)
    {
        lock_.lock();
    }

    ~MutexGuard()
    {
        lock_.unlock();
    }

    pthread_mutex_t* get_mutex()
    {
        return lock_.get_mutex();
    }

private:
    Mutex &lock_;
};

#endif //CPPTOOLKITS_MUTEXGUARD_H
