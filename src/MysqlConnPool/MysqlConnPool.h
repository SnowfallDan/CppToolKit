#ifndef MYUTILS_SQLHELPER_H
#define MYUTILS_SQLHELPER_H

#include <string>
#include <list>
#include <memory>
#include <mutex>

#include "MysqlConnection.h"

using std::string;
using std::list;

class MysqlConnPool : public noncopyable, public std::enable_shared_from_this<MysqlConnPool>
{
public:
    typedef std::shared_ptr<MysqlConnPool> Ptr;

    static MysqlConnPool::Ptr get_instance(string url, string user_name, string password, uint max_conn_size = 10, uint inc_step = 10);

    MysqlConnection::Ptr get_connection();

    void release_connection(const MysqlConnection::Ptr& conn);

    void terminate();

    uint get_current_size() const;

    Driver *get_driver();

    bool if_exceed_conn_limit();

private:
    MysqlConnPool(string url, string user_name, string password, uint max_conn_size, uint inc_step);

private:
    static MysqlConnPool::Ptr instance_;

    Driver *driver_;

    string conn_url_;
    string user_name_;
    string password_;
    uint current_conn_size_;
    uint inc_step_;
    uint max_conn_size_;

    list<MysqlConnection::Ptr> conn_list_;
    std::mutex conn_list_mutex_;

    bool if_exceed_conn_limit_ = false;
};


#endif //MYUTILS_SQLHELPER_H
