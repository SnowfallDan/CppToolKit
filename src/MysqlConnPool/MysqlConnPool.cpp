#include "MysqlConnPool.h"

namespace toolkit
{
MysqlConnPool::Ptr MysqlConnPool::instance_ = nullptr;

MysqlConnPool::Ptr
MysqlConnPool::get_instance()
{
    if (instance_.get() == nullptr)
        instance_.reset(new MysqlConnPool());
    return instance_;
}

// private
MysqlConnPool::MysqlConnPool()
{
    try
    {
        driver_ = get_driver_instance();
    }
    catch (sql::SQLException &e)
    {
        std::cout << "get_driver_instance() ec=" << e.getErrorCode() << ": " << e.what() << std::endl;
        throw;
    }
}

//template<typename... Args>
//void MysqlConnPool::init(Args &&... arg)
//{
//    pool_.reset(new PoolType(std::forward<Args>(arg)...));
//    pool_->obtain();
//}

Driver *MysqlConnPool::get_driver()
{
    return driver_;
}

void MysqlConnPool::set_pool_size(uint size)
{
    check_inited_();
    pool_->set_pool_size(size);
}

void MysqlConnPool::check_inited_()
{
    if (!pool_)
    {
        throw;
    }
}

int MysqlConnPool::execute_sql(string sql)
{
    typename PoolType::ResPtr conn;
    try
    {
        check_inited_();
        //捕获执行异常
        conn = pool_->obtain();
        return conn ? conn->create_statement_and_execute(sql) : -1;
    }
    catch (...)
    {
        conn->clean();
        conn.quit();
        throw;
    }
}

//template<typename Fmt, typename... Args>
//int MysqlConnPool::execute_sql(Fmt &&fmt, Args &&... arg)
//{
//    typename PoolType::ResPtr conn;
//    try
//    {
//        check_inited_();
//        //捕获执行异常
//        conn = pool_->obtain();
//        return conn ? conn->create_statement_and_execute(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...)) : -1;
//    }
//    catch (...)
//    {
//        conn->clean();
//        conn.quit();
//        throw;
//    }
//}

template<typename Fmt, typename... Args>
ResultSetPtr MysqlConnPool::query(Fmt &&fmt, Args &&... arg)
{
    typename PoolType::ResPtr conn;
    try
    {
        check_inited_();
        //捕获执行异常
        conn = pool_->obtain();
        if (conn)
            conn->prepare_statement_query(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...));
        return nullptr;
    }
    catch (...)
    {
        conn->clean();
        conn.quit();
        throw;
    }
}

template<typename ...Args>
string MysqlConnPool::query_string_(const char *fmt, Args &&...arg)
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
string MysqlConnPool::query_string_(const string &fmt, Args &&...args)
{
    return queryString(fmt.data(), std::forward<Args>(args)...);
}

const char *MysqlConnPool::query_string_(const char *fmt)
{
    return fmt;
}

const string &MysqlConnPool::query_string_(const string &fmt)
{
    return fmt;
}

}