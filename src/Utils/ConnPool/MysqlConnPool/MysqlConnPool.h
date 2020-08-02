#ifndef CPPTOOLKITS_SQLHELPER_H
#define CPPTOOLKITS_SQLHELPER_H

#include <string>
#include <list>
#include <memory>
#include <mutex>

#include "MysqlConnection.h"
#include "ResourcePool.h"

using std::string;
using std::list;

#define MYSQL_CONNECTION_POOL toolkit::MysqlConnPool::get_instance()
#define GET_MYSQL_CONNECTION() toolkit::MysqlConnPool::get_instance()->get_conn()

namespace toolkit
{

// Mysql数据库连接池, 基于工具类src/Utils/ResourcePool.h
class MysqlConnPool : public noncopyable, public std::enable_shared_from_this<MysqlConnPool>
{
public:
    typedef std::shared_ptr<MysqlConnPool> Ptr;
    typedef ResourcePool<MysqlConnection> PoolType;

    DISABLE_COPY_AND_ASSIGN(MysqlConnPool);

    static Ptr get_instance();

    /**
     * 初始化循环池，设置数据库连接参数
     * @tparam Args
     * @param arg
     */
    template<typename ...Args>
    void init(Args &&...arg)
    {
        pool_.reset(new PoolType(shared_from_this(), std::forward<Args>(arg)...));
        pool_->obtain();
    }

    /**
     * 获得一个Mysql Connection
     */
    SharedPtrRes<MysqlConnection> get_conn()
    {
        return pool_->obtain();
    }

    /**
     * 设置Mysql循环池对象个数
     * @param size
     */
    void set_pool_size(uint size);

    Driver *get_driver();

    void set_throw_ex_flag(bool flag)
    {
        throw_able_ = flag;
    }

    /**
     * 同步执行sql
     * @tparam Fmt
     * @tparam Args
     * @param fmt
     * @param arg
     */
    template<typename Fmt,typename ...Args>
    int execute_sql(Fmt &&fmt, Args && ...arg)
    {
        typename PoolType::ResPtr conn;
        try
        {
            check_inited_();
            //捕获执行异常
            conn = pool_->obtain();
            return conn ? conn->execute(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...)) : -1;
        }
        catch (...)
        {
            conn->clean();
            conn.quit();
            if(throw_able_)
                throw;
            return -1;
        }
    }

    /**
     * 同步执行查询
     * @tparam Fmt
     * @tparam Args
     * @param fmt
     * @param arg
     */
    template<typename Fmt,typename ...Args>
    ResultSetPtr query(Fmt &&fmt, Args && ...arg)
    {
        typename PoolType::ResPtr conn;
        try
        {
            check_inited_();
            //捕获执行异常
            conn = pool_->obtain();
            if (conn)
                conn->query(query_string_(std::forward<Fmt>(fmt), std::forward<Args>(arg)...));
            return conn->get_result_set();
        }
        catch (...)
        {
            conn->clean();
            conn.quit();
            if(throw_able_)
                throw;
            return nullptr;
        }
    }

private:
    MysqlConnPool();

    bool check_inited_();

private:
    static Ptr instance_;
    Driver *driver_;
    std::shared_ptr<PoolType> pool_;
    bool throw_able_ = false;
};

}

#endif //CPPTOOLKITS_SQLHELPER_H
