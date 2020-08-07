#ifndef CPPTOOLKITS_CONDITION_H
#define CPPTOOLKITS_CONDITION_H

#include <pthread.h>
#include <cstdio>

#include "Mutex.h"

class Condition : noncopyable
{
public:
    explicit Condition(Mutex &mutex)
            : mutex_(mutex),
              cond_()
    {
        auto ret = pthread_cond_init(&cond_, nullptr);
        assert(ret == 0);
    }

    ~Condition()
    {
        auto ret = pthread_cond_destroy(&cond_);
        assert(ret == 0);
    }

    void wait()
    {
        mutex_.unset_holder();
        auto ret = pthread_cond_wait(&cond_, mutex_.get_mutex());
        assert(ret == 0);
        printf("wake up");
        mutex_.set_holder();
    }

    void notify()
    {
        auto ret = pthread_cond_signal(&cond_);
        assert(ret == 0);
    }

    void notifyAll()
    {
        auto ret = pthread_cond_broadcast(&cond_);
        assert(ret == 0);
    }

private:
    Mutex &mutex_;
    pthread_cond_t cond_;

};


#endif //CPPTOOLKITS_CONDITION_H
