#include "MysqlConnPool.h"
#include "TimeFunc.h"

using namespace toolkit;

#define SIZE 30

void func(const MysqlConnPool::Ptr& conn_pool)
{
    auto begin = getCurrentMillisecond();
    auto end = getCurrentMillisecond();
}

int main()
{
    try
    {
        auto conn_pool = toolkit::MysqlConnPool::get_instance();
        // connection entry, db name, username, password
        toolkit::MysqlConnPool::get_instance()->init(conn_pool, "tcp://172.30.46.40:4000", "iceberg", "iceberg", "iceberg123");
        toolkit::MysqlConnPool::get_instance()->set_pool_size(thread::hardware_concurrency());
//        toolkit::MysqlConnPool::get_instance()->execute_sql("DROP TABLE IF EXISTS test");
        toolkit::MysqlConnPool::get_instance()->execute_sql("CREATE TABLE test(id INT, label CHAR(1))");
        toolkit::MysqlConnPool::get_instance()->execute_sql("INSERT INTO test(id, label) VALUES (1, 'a')");
    }
    catch (sql::SQLException &e)
    {
        /*
        MySQL Connector/C++ throws three different exceptions:

        - sql::MethodNotImplementedException (derived from sql::SQLException)
        - sql::InvalidArgumentException (derived from sql::SQLException)
        - sql::SQLException (derived from std::runtime_error)
        */
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        /* what() (derived from std::runtime_error) fetches error message */
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;

        return EXIT_FAILURE;
    }
    catch (...)
    {

    }


//    std::thread thread_pool[SIZE];
//    for (auto & thread : thread_pool)
//        thread = std::thread(&func, conn_pool);
//
//    for (auto & thread : thread_pool)
//        thread.join();
//
//    sleep(1);
//    for (auto & thread : thread_pool)
//        thread = std::thread(&func, conn_pool);
//
//    for (auto & thread : thread_pool)
//        thread.join();

    return 0;
}