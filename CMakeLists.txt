cmake_minimum_required(VERSION 3.14)
project(myUtils)
set(CMAKE_CXX_STANDARD 14)

MESSAGE( STATUS "PROJECT_NAME: " ${PROJECT_NAME} )

if (CMAKE_HOST_WIN32)
    set(WINDOWS 1)
elseif(CMAKE_HOST_APPLE)
    set(MACOS 1)
elseif(CMAKE_HOST_UNIX)
    set(LINUX 1)
endif()

# Add search path for include and lib files
if(WINDOWS)
    link_directories("${PROJECT_SOURCE_DIR}/platforms/win") 
elseif(LINUX)
    link_directories("${PROJECT_SOURCE_DIR}/platforms/linux") 
elseif(MACOS)
    link_directories("${PROJECT_SOURCE_DIR}/platforms/macos") 
endif()
include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")


include_directories(Log)
add_subdirectory(Log)

add_executable(myUtils main.cpp)
TARGET_LINK_LIBRARIES(myUtils spd_log)

# Set RPATH
if(WINDOWS)
INSTALL(TARGETS myUtils RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/out/")
endif()
