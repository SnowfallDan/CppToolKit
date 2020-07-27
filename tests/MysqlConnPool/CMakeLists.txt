add_executable(mysql_conn_test mysql_conn_test.cpp
        ../../src/ConnPool/MysqlConnPool/MysqlConnPool.cpp ../../src/ConnPool/MysqlConnPool/MysqlConnPool.h
        ../../src/ConnPool/MysqlConnPool/MysqlConnection.cpp ../../src/ConnPool/MysqlConnPool/MysqlConnection.h)
target_link_libraries(mysql_conn_test mysqlcppconn pthread)

add_executable(mysql_benchmark mysql_benchmark.cpp
        ../../src/ConnPool/MysqlConnPool/MysqlConnPool.cpp ../../src/ConnPool/MysqlConnPool/MysqlConnPool.h
        ../../src/ConnPool/MysqlConnPool/MysqlConnection.cpp ../../src/ConnPool/MysqlConnPool/MysqlConnection.h)
target_link_libraries(mysql_benchmark mysqlcppconn pthread)