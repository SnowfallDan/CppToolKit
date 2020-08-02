#include <memory>

#include "LRUQueue.h"
#include "LRUQueueWithMutex.h"
#include "TimeFunc.h"

#define LRU_QUEUE_SIZE 100
typedef LRUQueue<int, int> LRU;

// 1M times
#define INSERT_TIMES 1000000

int main()
{
    // LRU Init
    LRU lru(INSERT_TIMES);

    // LRU Insert 1M Times
    auto begin = Time::getCurrentMillisecond();
    for (int i = 0; i < INSERT_TIMES; ++i)
        lru.put(i, i);
    auto end = Time::getCurrentMillisecond();
    std::cout << "LRU insert 1M times cost: "<< end - begin << "ms" << std::endl;
    std::cout << "Each insert cost: "<< ((double)(end - begin) / (double)INSERT_TIMES) << "ms" << std::endl;

    // LRU Get 1M Times
    begin = Time::getCurrentMillisecond();
    for (int i = 0; i < INSERT_TIMES; ++i)
        lru.get(i);
    end = Time::getCurrentMillisecond();
    std::cout << "LRU get 1M times cost: "<< end - begin << "ms" << std::endl;
    std::cout << "Each get cost: "<< ((double)(end - begin) / (double)INSERT_TIMES) << "ms" << std::endl;

    return 0;
}
