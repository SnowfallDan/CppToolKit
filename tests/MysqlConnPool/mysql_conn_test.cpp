#include "MysqlConnPool.h"
#include "TimeFunc.h"

using namespace toolkit;
using namespace std;

#define SIZE 2

void func_insert(int n, const char *ch)
{
    auto begin = getCurrentMillisecond();
    toolkit::MysqlConnPool::get_instance()->execute_sql("INSERT INTO test(id, label) VALUES (%d, '%s')", n, ch);
    auto end = getCurrentMillisecond();
    cout << "[thread " << n  << "] insert cost: " << end - begin << "ms" << endl;
}

void func_select(int n)
{
    auto begin = getCurrentMillisecond();
    auto res = toolkit::MysqlConnPool::get_instance()->query("SELECT * FROM test WHERE id = %d", n);
    while (res && res->next())
    {
        // You can use either numeric offsets...
        cout << "id = " << res->getInt(1); // getInt(1) returns the first column
        // ... or column names for accessing results.
        // The latter is recommended.
        cout << ", label = '" << res->getString("label") << "'" << endl;
    }
    auto end = getCurrentMillisecond();
    cout << "[thread " << n  << "] select cost: " << end - begin << "ms" << endl;
}

int main()
{
    try
    {
        auto conn_pool = toolkit::MysqlConnPool::get_instance();
        // connection entry, db name, username, password
        toolkit::MysqlConnPool::get_instance()->init(conn_pool, "tcp://172.30.46.40:4000", "iceberg", "iceberg", "iceberg123");
        toolkit::MysqlConnPool::get_instance()->set_pool_size(thread::hardware_concurrency() * 10);

        toolkit::MysqlConnPool::get_instance()->execute_sql("DROP TABLE IF EXISTS test");
        toolkit::MysqlConnPool::get_instance()->execute_sql("CREATE TABLE test(id INT, label CHAR(10))");

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

    return 0;
}