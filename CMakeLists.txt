cmake_minimum_required(VERSION 3.14)
project(CppToolKits)
set(CMAKE_CXX_STANDARD 14)

MESSAGE( STATUS "PROJECT_NAME: " ${PROJECT_NAME} )

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(BuildType "Release")
    message(STATUS "Compile 'Release' Version")
else()
    set(BuildType "Debug")
    message(STATUS "Compile 'Debug' Version")
endif()

set(RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(LIBRARY_OUTPUT_PATH ${RELEASE_DIR}/linux/${BuildType})
    SET(EXECUTABLE_OUTPUT_PATH ${RELEASE_DIR}/linux/${BuildType})
    add_compile_options(-fPIC)
endif ()

set(CppToolKits_Src ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")

include_directories(src/Log)
include_directories(src/MemoryPool)
include_directories(src/Net/EventPoller)
include_directories(src/ConnPool/MysqlConnPool)
include_directories(src/Utils)
include_directories(src/DataStructure)

add_subdirectory(tests)
