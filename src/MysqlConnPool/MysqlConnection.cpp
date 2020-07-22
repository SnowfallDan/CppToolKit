#include "MysqlConnection.h"
#include "MysqlConnPool.h"

MysqlConnection::MysqlConnection(const std::shared_ptr<MysqlConnPool> &pool,
                                 string db_entry,
                                 string user_name,
                                 string password)
        : conn_(),
          db_entry_(std::move(db_entry)),
          user_name_(std::move(user_name)),
          password_(std::move(password))
{
    conn_pool_ = pool;

    init_conn_();
}

std::shared_ptr<MysqlConnPool> MysqlConnection::get_conn_pool()
{
    return conn_pool_.lock();
}

MysqlConnection::ConnectionPtr MysqlConnection::get_conn()
{
    return conn_;
}

void MysqlConnection::init_conn_()
{
    try
    {
        std::shared_ptr<MysqlConnPool> strong_pool_ptr(conn_pool_.lock());
        if(strong_pool_ptr && strong_pool_ptr->get_driver())
            conn_ = ConnectionPtr(strong_pool_ptr->get_driver()->connect(db_entry_, user_name_, password_));
    }
    catch (sql::SQLException &e)
    {
        std::cout << "connect() ec=" << e.getErrorCode() << ": " << e.what() << std::endl;
    }
}

bool MysqlConnection::is_closed()
{
    return conn_->isClosed();
}

bool MysqlConnection::is_valid()
{
    return conn_->isValid();
}

bool MysqlConnection::is_readonly()
{
    return conn_->isReadOnly();
}

void MysqlConnection::close()
{
    if (conn_)
    {
        try
        {
            conn_->close();
        }
        catch (sql::SQLException &e)
        {
            std::cout << "close() ec=" << e.getErrorCode() << ": " << e.what() << std::endl;
        }
    }
}
