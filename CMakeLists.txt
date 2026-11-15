cmake_minimum_required(VERSION 3.16)
project(tcp_ping LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(socket_utils STATIC "${SRC_DIR}/socket_utils.cpp")
target_include_directories(socket_utils PUBLIC "${INCLUDE_DIR}")


add_executable(server "${SRC_DIR}/server.cpp")
target_link_libraries(server PRIVATE socket_utils)

add_executable(client "${SRC_DIR}/client.cpp")
target_link_libraries(client PRIVATE socket_utils)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(tgt socket_utils server client)
        target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
endif()