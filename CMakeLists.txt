cmake_minimum_required(VERSION 3.16)
project(xproxy LANGUAGES CXX)


option(BUILD_TESTS "Build the tests" OFF)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # -std=c++17 not -std=gnu++17

# ---- clangd  compile_commands.json ----
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(/usr/local/include)
include_directories($ENV{HOME}/.local/include)

# ---- find libuv ----
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(UV QUIET libuv)
endif()

if(NOT UV_FOUND)
    message(FATAL_ERROR "libuv not found")
endif()


set(SOURCES
    src/uv_base.cpp
    src/stream_base.cpp
    src/datagram_base.cpp
    src/tcp_server.cpp
    src/tcp_client.cpp
    src/udp_server.cpp
    src/udp_client.cpp
    src/forward.cpp
    src/proxy_base.cpp
    src/proxy_tcp.cpp
    src/proxy_udp.cpp
)

add_executable(xproxy
    ${SOURCES}
    src/main.cpp
)

target_include_directories(xproxy PRIVATE ${UV_INCLUDE_DIRS})
target_link_libraries(xproxy PRIVATE ${UV_LIBRARIES})

if(UNIX AND NOT APPLE)
    target_link_libraries(xproxy PRIVATE pthread dl)
endif()


if(BUILD_TESTS)
enable_testing()
add_library(proxytest ${SOURCES})
target_compile_definitions(proxytest PRIVATE LOGGER_ASYNC_ENABLED)
add_subdirectory(test)
endif()
