add_executable(test_uuid test_uuid.cpp)
target_link_libraries(test_uuid uuid)

add_executable(test_backtrace test_backtrace.cpp)

add_executable(test_wrap_malloc test_wrap_malloc.cpp)
target_link_libraries(test_wrap_malloc ${LINK_LIB_LIST})

