set(MysqlConnPool_src ${CppToolKits_Src}/Utils/ConnPool/MysqlConnPool)
file(GLOB mysql_conn_list ${MysqlConnPool_src}/*.cpp ${MysqlConnPool_src}/*.h)
add_library(mysqlconnpool STATIC ${mysql_conn_list})