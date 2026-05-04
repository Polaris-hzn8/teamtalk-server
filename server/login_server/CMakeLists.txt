cmake_minimum_required(VERSION 3.10)
project(login_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../.sdk")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

file(GLOB_RECURSE LOGIN_SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(login_server ${LOGIN_SERVER_SOURCES})

target_compile_options(login_server PRIVATE -g -W -Wall)
target_compile_definitions(login_server PRIVATE
    _REENTRANT
    _FILE_OFFSET_BITS=64
    AC_HAS_INFO
    AC_HAS_WARNING
    AC_HAS_ERROR
    AC_HAS_CRITICAL
    AC_HAS_DEBUG
    TIXML_USE_STL
    LINUX_DAEMON
)

link_directories(
    "${SDK_DIR}/teamtalk-imcore/lib"
    "${SDK_DIR}/protobuf/lib"
    "${SDK_DIR}/teamtalk-sbase/lib"
)

target_include_directories(login_server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${SDK_DIR}/teamtalk-imcore/include"
    "${SDK_DIR}/teamtalk-sbase/include"
    "${SDK_DIR}/protobuf/include"
)

target_link_libraries(login_server PRIVATE
    teamtalk-sbase
    teamtalk-imcore
    protobuf-lite
    pthread
)
