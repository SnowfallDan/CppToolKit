#include "wrap_malloc.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <functional>

class Test
{
public:
    explicit Test(int numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
            threads_.emplace_back(new std::thread(std::bind(&Test::threadFunc, this)));
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
        // Leak some memory.
        malloc(100);
    }

    std::vector<std::unique_ptr<std::thread>> threads_;
};

int main()
{
    Test t(1);
    t.joinAll();

    dump_malloc_info();

    return 0;
}