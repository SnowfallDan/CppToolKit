#include "MysqlConnPool.h"

MysqlConnPool::Ptr MysqlConnPool::instance_ = nullptr;

MysqlConnPool::Ptr MysqlConnPool::get_instance(string url, string user_name, string password, uint max_conn_size, uint inc_step)
{
    if (instance_.get() == nullptr)
        instance_.reset(new MysqlConnPool(std::move(url), std::move(user_name), std::move(password), max_conn_size, inc_step));
    return instance_;
}

MysqlConnection::Ptr MysqlConnPool::get_connection()
{
    MysqlConnection::Ptr conn;
    std::lock_guard<std::mutex> lock(conn_list_mutex_);
    if (!conn_list_.empty())
    {
        printf("thread: %d, conn_list_ not empty\n", ::gettid());
        conn = conn_list_.front();
        conn_list_.pop_front();

        if (conn->is_closed())
            conn.reset(new MysqlConnection(shared_from_this(), conn_url_, user_name_, password_));
    }
    else
    {
        printf("thread: %d, conn_list_  empty\n", ::gettid());
        for (int i = 0; i < inc_step_; ++i)
        {
            conn = std::make_shared<MysqlConnection>(shared_from_this(), conn_url_, user_name_, password_);
            conn_list_.push_back(conn);
        }
        conn = std::make_shared<MysqlConnection>(shared_from_this(), conn_url_, user_name_, password_);
    }

    if ( ++current_conn_size_ > max_conn_size_ && !if_exceed_conn_limit_)
        if_exceed_conn_limit_ = true;

    printf("thread: %d, get conn\n", ::gettid());
    return conn;
}

void MysqlConnPool::release_connection(const MysqlConnection::Ptr &conn)
{
    if (conn)
    {
        std::lock_guard<std::mutex> lock(conn_list_mutex_);
        current_conn_size_--;
        printf("thread: %d, release_connection\n", ::gettid());
        if (current_conn_size_ < max_conn_size_)
            conn_list_.push_back(conn);

        if(if_exceed_conn_limit_ && current_conn_size_ == max_conn_size_)
            if_exceed_conn_limit_ = false;
    }
    printf("thread: %d, release_connection end\n", ::gettid());
}

void MysqlConnPool::terminate()
{
    std::lock_guard<std::mutex> lock(conn_list_mutex_);
    for (auto &it : conn_list_)
    {
        it->close();
    }
    conn_list_.clear();
    current_conn_size_ = 0;
}

uint MysqlConnPool::get_current_size() const
{
    return current_conn_size_;
}

// private
MysqlConnPool::MysqlConnPool(string url, string user_name, string password, uint max_conn_size, uint inc_step)
        : conn_url_(std::move(url)),
          user_name_(std::move(user_name)),
          password_(std::move(password)),
          current_conn_size_(0),
          inc_step_(inc_step),
          max_conn_size_(max_conn_size),
          conn_list_()
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

bool MysqlConnPool::if_exceed_conn_limit()
{
    return if_exceed_conn_limit_;
}
