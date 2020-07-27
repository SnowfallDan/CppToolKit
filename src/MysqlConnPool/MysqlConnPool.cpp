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

}