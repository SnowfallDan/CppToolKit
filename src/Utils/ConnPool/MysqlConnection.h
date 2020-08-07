#ifndef CPPTOOLKITS_MYSQLCONNECTION_H
#define CPPTOOLKITS_MYSQLCONNECTION_H

#include <memory>
#include <string>
#include <vector>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "noncopyable.h"
#include "StringUtils.h"
#include "DisableCopyAndAssign.h"

using std::string;
using sql::Connection;
using sql::Driver;
using sql::Statement;
using sql::PreparedStatement;
using sql::ResultSet;
using std::vector;

namespace toolkit
{

typedef std::shared_ptr<Connection> ConnectionPtr;
typedef std::shared_ptr<Statement> StatementPtr;
typedef std::shared_ptr<PreparedStatement> PreparedStatementPtr;
typedef std::shared_ptr<ResultSet> ResultSetPtr;

class MysqlConnPool;

class MysqlConnection : public noncopyable
{
public:
    typedef std::shared_ptr<MysqlConnection> Ptr;

    explicit MysqlConnection(const std::shared_ptr<MysqlConnPool> &pool,
                             string db_entry,
                             string db_name,
                             string user_name,
                             string password);

    ~MysqlConnection();

    DISABLE_COPY_AND_ASSIGN(MysqlConnection);

    std::shared_ptr<MysqlConnPool> get_conn_pool();

    ConnectionPtr get_conn();

    bool is_closed();

    void set_auto_commit(bool flag);

    void close();

    bool is_valid();

    bool is_readonly();

    bool reconnect();

    int execute(const string &sql);

    int query(const string &sql);

    ResultSetPtr& get_result_set();

    // transaction
    void prepare_transaction();

    template<typename Fmt, typename... Args>
    void add_transaction_sql(Fmt &&fmt, Args &&... arg)
    {
        transaction_sql_vec_.push_back(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...));
    }

    bool execute_transaction(bool if_throw = false);

    void clean();

    void check();

private:
    void init_conn_();

private:
    std::weak_ptr<MysqlConnPool> conn_pool_;

    sql::ConnectOptionsMap connection_properties_;

    ConnectionPtr conn_;
    StatementPtr stmt_;
    PreparedStatementPtr pstmt_;
    ResultSetPtr res_;

    string db_entry_;
    string db_name_;
    string user_name_;
    string password_;

    string sql_;
    bool if_auto_commit_ = true;

    vector<string> transaction_sql_vec_;
};

}

#endif //CPPTOOLKITS_MYSQLCONNECTION_H
