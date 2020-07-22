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

class MysqlConnPool;

class MysqlConnection : public noncopyable
{
public:
    typedef std::shared_ptr<Connection> ConnectionPtr;
    typedef std::shared_ptr<MysqlConnection> Ptr;

    explicit MysqlConnection(const std::shared_ptr<MysqlConnPool> &pool,
                             string db_entry,
                             string user_name,
                             string password);

    std::shared_ptr<MysqlConnPool> get_conn_pool();

    ConnectionPtr get_conn();

    bool is_closed();

    void close();

    bool is_valid();

    bool is_readonly();

private:
    void init_conn_();

private:
    std::weak_ptr<MysqlConnPool> conn_pool_;

    ConnectionPtr conn_;

    string db_entry_;
    string user_name_;
    string password_;
};


#endif //MYUTILS_MYSQLCONNECTION_H
