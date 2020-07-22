add_executable(mysql_conn_test ../MysqlConnPool/mysql_conn_test.cpp
        ../../src/MysqlConnPool/MysqlConnPool.cpp ../../src/MysqlConnPool/MysqlConnPool.h
        ../../src/MysqlConnPool/MysqlConnection.cpp ../../src/MysqlConnPool/MysqlConnection.h)
target_link_libraries(mysql_conn_test mysqlcppconn pthread)