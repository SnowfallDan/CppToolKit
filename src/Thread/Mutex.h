#ifndef CPPTOOLKITS_MUTEX_H
#define CPPTOOLKITS_MUTEX_H

#include <pthread.h>
#include <cassert>

#include "CurrentThread.h"
#include "noncopyable.h"
#include "disableCopyAndAssign.h"

class Mutex : noncopyable
{
public:
    DISABLE_COPY_AND_ASSIGN(Mutex);

    Mutex() : mutex_holder_(0)
    {
        auto ret = pthread_mutex_init(&mutex_, nullptr);
        assert(ret == 0);
    }

    ~Mutex()
    {
        assert(mutex_holder_ == 0);
        auto ret = pthread_mutex_destroy(&mutex_);
        assert(ret == 0);
    }

    void lock()
    {
        auto ret = pthread_mutex_lock(&mutex_);
        assert(ret == 0);
        set_holder();
    }

    void unlock()
    {
        unset_holder();
        auto ret = pthread_mutex_unlock(&mutex_);
        assert(ret == 0);
    }

    bool try_lock()
    {
        return pthread_mutex_trylock(&mutex_) == 0;
    }

    pthread_mutex_t* get_mutex()
    {
        return &mutex_;
    }

    bool is_current_thread() const
    {
        return mutex_holder_ == CurrentThread::which_tid();
    }

    void unset_holder()
    {
        mutex_holder_ = 0;
    }

    void set_holder()
    {
        mutex_holder_ = CurrentThread::which_tid();
    }

private:
    pthread_mutex_t mutex_;
    int mutex_holder_;
};

#endif //CPPTOOLKITS_MUTEX_H
