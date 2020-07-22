#include "MysqlConnPool.h"
#include "TimeFunc.h"

#define SIZE 30

void func(const MysqlConnPool::Ptr& conn_pool)
{
    auto begin = getCurrentMillisecond();
    auto conn = conn_pool->get_connection();
    auto end = getCurrentMillisecond();
    if(conn->is_valid())
        printf("thread: %d, genrate %d mysql connection! cost = %lums\n", ::gettid(), conn_pool->get_current_size(), end - begin);
    conn_pool->release_connection(conn);
}


int main()
{
    auto conn_pool = MysqlConnPool::get_instance("tcp://172.30.46.40:4000", "iceberg", "iceberg123", 10, 10);
//    func(conn_pool);
    std::thread thread_pool[SIZE];
    for (auto & thread : thread_pool)
        thread = std::thread(&func, conn_pool);

    for (auto & thread : thread_pool)
        thread.join();

    sleep(1);
    for (auto & thread : thread_pool)
        thread = std::thread(&func, conn_pool);

    for (auto & thread : thread_pool)
        thread.join();

    conn_pool->terminate();
    return 0;
}