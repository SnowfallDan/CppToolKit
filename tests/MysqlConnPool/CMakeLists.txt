set(MysqlConnPool_src ${CppToolKits_Src}/Utils/ConnPool/MysqlConnPool)
file(GLOB mysql_conn_list ${MysqlConnPool_src}/*.cpp ${MysqlConnPool_src}/*.h )
add_library(mysqlconnpool STATIC ${mysql_conn_list})

add_executable(mysql_conn_test mysql_conn_test.cpp)
target_link_libraries(mysql_conn_test mysqlconnpool mysqlcppconn pthread)

add_executable(mysql_benchmark mysql_benchmark.cpp)
target_link_libraries(mysql_benchmark mysqlconnpool mysqlcppconn pthread)