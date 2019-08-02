cmake_minimum_required(VERSION 3.14)
project(myUtils)
set(CMAKE_CXX_STANDARD 14)

include_directories(Log)
add_subdirectory(Log)

add_executable(myUtils main.cpp)

TARGET_LINK_LIBRARIES(myUtils spd_log)
