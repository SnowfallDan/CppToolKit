#ifndef CPPTOOLKITS_COUNTDOWNLATCH_H
#define CPPTOOLKITS_COUNTDOWNLATCH_H

#include "Condition.h"
#include "MutexGuard.h"

namespace toolkit
{

class CountDownLatch : noncopyable
{
public:
    explicit CountDownLatch(int count)
            : mutex_(),
              condition_(mutex_),
              count_(count)
    {

    }

    void wait()
    {
        MutexGuard lock(mutex_);
        while (count_ > 0)
        {
            condition_.wait();
        }
    }

    void count_down()
    {
        MutexGuard lock(mutex_);
        --count_;
        if (count_ == 0)
        {
            condition_.notifyAll();
        }
    }

    int get_count() const
    {
        MutexGuard lock(mutex_);
        return count_;
    }

private:
    mutable Mutex mutex_;
    Condition condition_;
    int count_;
};

}

#endif //CPPTOOLKITS_COUNTDOWNLATCH_H
