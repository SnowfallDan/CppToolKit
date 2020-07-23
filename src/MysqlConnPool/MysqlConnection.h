#ifndef MYUTILS_MYSQLCONNECTION_H
#define MYUTILS_MYSQLCONNECTION_H

#include <memory>
#include <string>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "noncopyable.h"

using std::string;
using sql::Connection;
using sql::Driver;
using sql::Statement;
using sql::PreparedStatement;
using sql::ResultSet;

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

    std::shared_ptr<MysqlConnPool> get_conn_pool();

    ConnectionPtr get_conn();

    bool is_closed();

    void close();

    bool is_valid();

    bool is_readonly();

    bool reconnect();

    int create_statement_and_execute(const string &sql);

    int prepare_statement_query(const string &sql);

    ResultSetPtr& get_result_set();

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
};

}

#endif //MYUTILS_MYSQLCONNECTION_H
