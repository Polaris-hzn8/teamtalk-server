cmake_minimum_required(VERSION 3.10)
project(http_msg_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../.sdk)

file(GLOB_RECURSE SOURCE_LIST
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

link_directories(
    ${SDK_DIR}/teamtalk-imcore/lib
    ${SDK_DIR}/teamtalk-sbase/lib
    ${SDK_DIR}/protobuf/lib
    ${SDK_DIR}/jsoncpp/lib
)

add_executable(http_msg_server ${SOURCE_LIST})

target_include_directories(http_msg_server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SDK_DIR}/teamtalk-imcore/include"
    "${SDK_DIR}/teamtalk-sbase/include"
    "${SDK_DIR}/protobuf/include"
    "${SDK_DIR}/jsoncpp/include"
)

target_compile_options(http_msg_server PRIVATE -g -W -Wall)
target_compile_definitions(http_msg_server PRIVATE
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

target_link_libraries(http_msg_server PRIVATE
    teamtalk-sbase
    teamtalk-imcore
    protobuf-lite
    jsoncpp
    pthread
    curl
)

