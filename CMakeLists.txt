cmake_minimum_required(VERSION 3.14)

project(CppToolKits)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

set(CMAKE_CXX_FLAGS "-g3 -std=c++11")

option(ENABLETCMALLOC "Enable Google TCMalloc" OFF)
option(ENABLEJEMALLOC "Enable Facebook JeMalloc" OFF)
option(ENABLEWRAPMALLOC "Enable Wrap Malloc" ON)

#检查是否使用wrap malloc
if(ENABLEWRAPMALLOC)
    set(CMAKE_CXX_FLAGS "-Wl,--wrap,malloc -Wl,--wrap,free")
    message(STATUS "Use Wrap Malloc")
endif()

#查找google-perftools
find_package(TCMALLOC QUIET)
if (ENABLEJEMALLOC AND TCMALLOC_FOUND)
    message(STATUS "found library:${TCMALLOC_LIBRARIES}, ENABLE_TCMALLOC defined")
    include_directories(${TCMALLOC_INCLUDE_DIR})
    add_definitions(-DENABLE_TCMALLOC)
    list(APPEND LINK_LIB_LIST ${TCMALLOC_LIBRARIES})
endif()

#查找facebook-jemalloc
find_package(JEMALLOC QUIET)
if(ENABLETCMALLOC AND JEMALLOC_FOUND)
    message(STATUS "found library:${JEMALLOC_LIBRARIES}, ENABLE_JEMALLOC defined")
    add_definitions(-DENABLE_JEMALLOC)
    include_directories(${JEMALLOC_INCLUDE_DIR})
    list(APPEND LINK_LIB_LIST ${JEMALLOC_LIBRARIES})
endif()

set(CppToolKits_Src ${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")
include_directories(src/Log)
include_directories(src/MemoryPool)
include_directories(src/Net/EventPoller)
include_directories(src/ConnPool/MysqlConnPool)
include_directories(src/Utils)
include_directories(src/DataStructure)
include_directories(src/Thread)

list(APPEND LINK_LIB_LIST pthread)

add_definitions(-DHAVE_SYMBOLIZE)

add_subdirectory(tests)
