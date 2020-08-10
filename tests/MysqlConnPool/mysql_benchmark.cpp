#include "ConnPool/MysqlConnPool.h"
#include "timeFunc.h"

#ifdef ENABLE_WRAP_MALLOC
#include "wrap_malloc.h"
#endif

using namespace toolkit;
using namespace std;

#define SIZE 20

void func_insert(int n, const char *ch)
{
    auto begin = Time::getCurrentMillisecond();
    MYSQL_CONNECTION_POOL->execute_sql("INSERT INTO test(id, label) VALUES (%d, '%s')", n, ch);
    auto end = Time::getCurrentMillisecond();
    cout << "[thread " << n  << "] insert cost: " << end - begin << "ms" << endl;
}

void func_select(int n)
{
    auto begin = Time::getCurrentMillisecond();
    auto res = MYSQL_CONNECTION_POOL->query("SELECT * FROM test WHERE id = %d", n);
    while (res && res->next())
    {
        // You can use either numeric offsets...
        cout << "id = " << res->getInt(1); // getInt(1) returns the first column
        // ... or column names for accessing results.
        // The latter is recommended.
        cout << ", label = '" << res->getString("label") << "'" << endl;
    }
    auto end = Time::getCurrentMillisecond();
    cout << "[thread " << n  << "] select cost: " << end - begin << "ms" << endl;
}

int main()
{
    try
    {
        // connection entry, db name, username, password
        MYSQL_CONNECTION_POOL->init("tcp://172.30.46.40:4000", "iceberg", "iceberg", "iceberg123");
        MYSQL_CONNECTION_POOL->set_pool_size(thread::hardware_concurrency() * 10);

        MYSQL_CONNECTION_POOL->execute_sql("DROP TABLE IF EXISTS test");
        MYSQL_CONNECTION_POOL->execute_sql("CREATE TABLE test(id INT, label CHAR(10))");

        std::thread thread_pool[SIZE];
        int i = 0;
        for (auto & thread : thread_pool)
            thread = std::thread(&func_insert, ++i, std::to_string(i).c_str());

        for (auto & thread : thread_pool)
            thread.join();

        getchar();

        i = 0;
        for (auto & thread : thread_pool)
            thread = std::thread(&func_select, ++i);

        for (auto & thread : thread_pool)
            thread.join();

        getchar();
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
        cout << "# ERR: Unknow Exception!" << endl;
    }
#ifdef ENABLE_WRAP_MALLOC
    dump_malloc_info();
#endif
    return 0;
}

