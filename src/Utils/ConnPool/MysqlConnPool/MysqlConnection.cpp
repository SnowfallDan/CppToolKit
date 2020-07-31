#include "MysqlConnection.h"
#include "MysqlConnPool.h"

namespace toolkit
{
MysqlConnection::MysqlConnection(const std::shared_ptr<MysqlConnPool> &pool,
                                 string db_entry,
                                 string db_name,
                                 string user_name,
                                 string password)
        : conn_(),
          connection_properties_(),
          db_entry_(std::move(db_entry)),
          db_name_(std::move(db_name)),
          user_name_(std::move(user_name)),
          password_(std::move(password)),
          sql_()
{
    conn_pool_ = pool;

    init_conn_();
}

MysqlConnection::~MysqlConnection()
{
    clean();
}

std::shared_ptr<MysqlConnPool> MysqlConnection::get_conn_pool()
{
    return conn_pool_.lock();
}

ConnectionPtr MysqlConnection::get_conn()
{
    return conn_;
}

void MysqlConnection::init_conn_()
{
    try
    {
        std::shared_ptr<MysqlConnPool> strong_pool_ptr(conn_pool_.lock());
        if (strong_pool_ptr && strong_pool_ptr->get_driver())
        {
            connection_properties_["hostName"] = db_entry_;
            connection_properties_["userName"] = user_name_;
            connection_properties_["password"] = password_;
            connection_properties_["schema"] = db_name_;
            connection_properties_["OPT_RECONNECT"] = true;

            // Create a connection
            conn_ = ConnectionPtr(strong_pool_ptr->get_driver()->connect(connection_properties_));
            // Connect to the MySQL 'db_name_' database
            conn_->setSchema(db_name_);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cout << "connect() ec=" << e.getErrorCode() << ": " << e.what() << std::endl;
        throw;
    }
}

bool MysqlConnection::is_closed()
{
    return conn_->isClosed();
}

void MysqlConnection::set_auto_commit(bool flag)
{
    if_auto_commit_ = flag;
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

bool MysqlConnection::reconnect()
{
    return conn_->reconnect();
}

int MysqlConnection::execute(const string &sql)
{
    clean();
    sql_ = sql;
    int ec = 0;
    try
    {
        if(!if_auto_commit_)
            conn_->setAutoCommit(true);

        check();
        stmt_.reset(conn_->createStatement());
        if (stmt_ != nullptr)
            stmt_->execute(sql);
        return ec;
    }
    catch (sql::SQLException &e)
    {
        ec = e.getErrorCode();
        std::cout << "execute(sql) ec=" << ec << ": " << e.what() << std::endl;
        if (!conn_->isValid())
            reconnect();
        throw;
    }
}

int MysqlConnection::query(const string &sql)
{
    clean();
    sql_ = sql;
    int ec = 0;
    try
    {
        if(!if_auto_commit_)
            conn_->setAutoCommit(true);

        check();
        stmt_.reset(conn_->createStatement());
        if (stmt_ != nullptr)
            res_.reset(stmt_->executeQuery(sql_));
        return ec;
    }
    catch (sql::SQLException &e)
    {
        ec = e.getErrorCode();
        std::cout << "execute(sql) ec=" << ec << ": " << e.what() << std::endl;
        if (!conn_->isValid())
            reconnect();
        throw;
    }

}

ResultSetPtr &MysqlConnection::get_result_set()
{
    return res_;
}

void MysqlConnection::prepare_transaction()
{
    if(if_auto_commit_)
        set_auto_commit(false);
}

bool MysqlConnection::execute_transaction(bool if_throw)
{
    int ec = 0;
    try
    {
        for(auto &sql : transaction_sql_vec_)
        {
            pstmt_.reset(conn_->prepareStatement(sql));
            pstmt_->executeUpdate();
        }

        conn_->commit();
        return true;
    }
    catch (sql::SQLException &e)
    {
        conn_->rollback();
        ec = e.getErrorCode();
        std::cout << "execute(sql) ec=" << ec << ": " << e.what() << std::endl;
        if (!conn_->isValid())
            reconnect();

        if(if_throw)
            throw ;
        else
            return false;
    }
}

void MysqlConnection::clean()
{
    res_.reset();
    sql_.clear();
    transaction_sql_vec_.clear();
}

void MysqlConnection::check()
{
    if (!is_valid() && is_closed() && !conn_ && reconnect() && !conn_ && !is_valid() && is_closed())
        throw sql::SQLException("mysql lost connection!");
}

}