#ifndef CPPTOOLKITS_LRUQUEUEWITHMUTEX_H
#define CPPTOOLKITS_LRUQUEUEWITHMUTEX_H

#include <mutex>

#include "LRUQueue.h"

template<typename K, typename V>
class LRUQueueWithMutex
{
public:
    explicit LRUQueueWithMutex(unsigned int fixedCapacity = 1024) : lru_(fixedCapacity) {}

    template <class...Args>
    void put(const K &key, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(lru_mutex_);
        lru_.put(key, std::forward<Args>(args)...);
    }

    std::shared_ptr<V> get(const K &key)
    {
        std::lock_guard<std::mutex> lock(lru_mutex_);
        return lru_.get(key);
    }

private:
    LRUQueue<K, V> lru_;
    std::mutex lru_mutex_;
};

#endif //CPPTOOLKITS_LRUQUEUEWITHMUTEX_H
