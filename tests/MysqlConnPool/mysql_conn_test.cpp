#include "ConnPool/MysqlConnPool.h"
#include "TimeFunc.h"

#ifdef ENABLE_WRAP_MALLOC
#include "wrap_malloc.h"
#endif

using namespace toolkit;
using namespace std;

int main()
{
    try
    {
        // INIT MYSQL CONNECTION
        MYSQL_CONNECTION_POOL->init("tcp://172.30.46.40:4000", "iceberg", "iceberg", "iceberg123");
        // INIT POOL SIZE
        MYSQL_CONNECTION_POOL->set_pool_size(thread::hardware_concurrency() * 10);
        // EXECUTE DROP OR CREATE
        MYSQL_CONNECTION_POOL->execute_sql("DROP TABLE IF EXISTS test");
        MYSQL_CONNECTION_POOL->execute_sql("CREATE TABLE test(id INT, label CHAR(10))");
        // INSERT
        char value[1] = {'a'};
        MYSQL_CONNECTION_POOL->execute_sql("INSERT INTO test(id, label) VALUES (%d, '%s')", 1, value);
        //UPDATE
        MYSQL_CONNECTION_POOL->execute_sql("UPDATE test SET label='b' WHERE id=%d", 1);
        // SELECT
        auto res = MYSQL_CONNECTION_POOL->query("SELECT * FROM test WHERE id = %d", 1);
        while (res && res->next())
        {
            cout << "id = " << res->getInt(1); // getInt(1) returns the first column
            cout << ", label = '" << res->getString("label") << "'" << endl;
        }

        // TRANSACTION OK
        auto conn = GET_MYSQL_CONNECTION();
        conn->prepare_transaction();
        char tran_ok[1] = {'z'};
        conn->add_transaction_sql("INSERT INTO test(id, label) VALUES (%d, '%s')", 10, tran_ok);
        conn->add_transaction_sql("UPDATE test SET label='y' WHERE id=%d", 10);
        // not throw exception
        if(conn->execute_transaction())
        {
            res = MYSQL_CONNECTION_POOL->query("SELECT * FROM test WHERE id = %d", 10);
            while (res && res->next())
            {
                cout << "id = " << res->getInt(1);
                cout << ", label = '" << res->getString("label") << "'" << endl;
            }
        }
        else
            cout << "Transaction Commit Error!" << endl;

        // TRANSACTION ERROR
        conn = GET_MYSQL_CONNECTION();
        conn->prepare_transaction();
        char tran_err[1] = {'t'};
        conn->add_transaction_sql("INSERT INTO test(id, label) VALUES (%d, '%s')", 100, tran_err);
        conn->add_transaction_sql("UPDATE test SET label='y' WHERE id=%d", 10000);
        conn->add_transaction_sql("CREATE TABLE test(id INT, label CHAR(10))");
        // throw exception
        conn->execute_transaction(true);
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