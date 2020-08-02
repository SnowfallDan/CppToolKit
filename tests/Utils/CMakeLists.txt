add_executable(uuid_test uuid_test.cpp)
target_link_libraries(uuid_test uuid)

#set(CMAKE_CXX_FLAGS "-g3 -Wall -Wl,--wrap,malloc -Wl,--wrap,free")
add_executable(test_wrap_malloc test_wrap_malloc.cpp)
target_link_libraries(test_wrap_malloc ${LINK_LIB_LIST})