#ifndef MYUTILS_SQLHELPER_H
#define MYUTILS_SQLHELPER_H

#include <string>
#include <list>
#include <memory>
#include <mutex>

#include "MysqlConnection.h"
#include "ResourcePool.h"

using std::string;
using std::list;

namespace toolkit
{
// Mysql数据库连接池, 基于工具类src/Utils/ResourcePool.h
class MysqlConnPool : public noncopyable, public std::enable_shared_from_this<MysqlConnPool>
{
public:
    typedef std::shared_ptr<MysqlConnPool> Ptr;
    typedef ResourcePool<MysqlConnection> PoolType;

    static Ptr get_instance();

    template<typename ...Args>
    void init(Args &&...arg)
    {
        pool_.reset(new PoolType(std::forward<Args>(arg)...));
        pool_->obtain();
    }

    void set_pool_size(uint size);

    Driver *get_driver();

//    int execute_sql(string sql);

    template<typename Fmt,typename ...Args>
    int execute_sql(Fmt &&fmt, Args && ...arg)
    {
        typename PoolType::ResPtr conn;
        try
        {
            check_inited_();
            //捕获执行异常
            conn = pool_->obtain();
            return conn ? conn->create_statement_and_execute(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...)) : -1;
        }
        catch (...)
        {
            conn->clean();
            conn.quit();
            throw;
        }
    }

    template<typename Fmt,typename ...Args>
    ResultSetPtr query(Fmt &&fmt, Args && ...arg);

private:
    MysqlConnPool();

    void check_inited_();

    template<typename ...Args>
    static inline string query_string_(const char *fmt, Args &&...arg)
    {
        char *ptr_out = nullptr;
        asprintf(&ptr_out, fmt, arg...);
        if (ptr_out)
        {
            string ret(ptr_out);
            free(ptr_out);
            return ret;
        }
        return "";
    }

    template<typename ...Args>
    static inline string query_string_(const string &fmt, Args &&...args)
    {
        return queryString(fmt.data(), std::forward<Args>(args)...);
    }

    static inline const char *query_string_(const char *fmt)
    {
        return fmt;
    }

    static inline const string &query_string_(const string &fmt)
    {
        return fmt;
    }

private:
    static Ptr instance_;
    Driver *driver_;
    std::shared_ptr<PoolType> pool_;
};

}

#endif //MYUTILS_SQLHELPER_H
