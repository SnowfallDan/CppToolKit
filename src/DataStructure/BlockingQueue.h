#ifndef CPPTOOLKITS_BLOCKINGQUEUE_H
#define CPPTOOLKITS_BLOCKINGQUEUE_H

#include <deque>

#include "noncopyable.h"

using std::deque;

template<typename T>
class BlockingQueue : noncopyable
{
public:
    BlockingQueue() : mutex_(), not_emptry_cond_(mutex_.mutex()), queue_() {}

    void put(const T &one)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(one);
        not_emptry_cond_.notify_one();
    }

    void put(T&& one)
    {
        MutexLockGuard lock(mutex_);
        queue_.push_back(std::move(one));
        not_emptry_cond_.notify_one();
    }

    T take()
    {
        MutexLockGuard lock(mutex_);
        while (queue_.empty())
            not_emptry_cond_.wait();
        T one(std::move(queue_.front()));
        queue_.pop_front();
        return one;
    }

    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }

private:
    mutable Mutex mutex_;
    std::condition_variable not_emptry_cond_;
    deque<T> queue_;
};

#endif //CPPTOOLKITS_BLOCKINGQUEUE_H
