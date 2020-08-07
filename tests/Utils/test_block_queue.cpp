#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <functional>
#include <string>

#include "BlockingQueue.h"
#include "CountDownLatch.h"

using std::string;
using namespace toolkit;

class BlockQueueTest
{
public:
    explicit BlockQueueTest(int numThreads)
            : latch_(numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.emplace_back(new std::thread(std::bind(&BlockQueueTest::threadFunc, this), string(name)));
        }
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "hello %d", i);
            queue_.put(buf);
            printf("tid=%d, put data = %s, size = %zd\n", CurrentThread::which_tid(), buf, queue_.size());
        }
    }

    void joinAll()
    {
        for (auto& thr : threads_)
        {
            thr->join();
        }
    }

private:

    void threadFunc()
    {
        printf("tid=%d started\n",CurrentThread::which_tid());

        latch_.count_down();
        bool running = true;
        while (running)
        {
            std::string d(queue_.take());
            printf("tid=%d, get data = %s, size = %zd\n", CurrentThread::which_tid(), d.c_str(), queue_.size());
            running = (d != "stop");
        }

        printf("tid=%d, stopped\n", CurrentThread::which_tid());
    }

    BlockingQueue<std::string> queue_;
    CountDownLatch latch_;
    std::vector<std::unique_ptr<std::thread>> threads_;
};

void testMove()
{
    BlockingQueue<std::unique_ptr<int>> queue;
    queue.put(std::unique_ptr<int>(new int(42)));
    std::unique_ptr<int> x = queue.take();
    printf("took %d\n", *x);
    *x = 123;
    queue.put(std::move(x));
    std::unique_ptr<int> y = queue.take();
    printf("took %d\n", *y);
}

int main()
{
    printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::which_tid());
    BlockQueueTest t(5);
    t.run(100);
    t.joinAll();

    testMove();

    printf("number of created threads\n");
}
