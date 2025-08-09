cmake_minimum_required(VERSION 2.6)
project(db_proxy_server)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST1)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/redis SRC_LIST2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/business SRC_LIST3)

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base)
set(THIRD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 编译链接的库文件
if(APPLE)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
elseif(UNIX)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
endif()
set(SLOG_LIB_DIR ${THIRD_DIR}/slog/lib)
set(BASE_LIB_DIR ${BASE_DIR}/bin)
set(MYSQL_LIB_DIR /usr/lib64/mysql)

add_definitions(
    -g -W -Wall
    -D_REENTRANT
    -D_FILE_OFFSET_BITS=64
    -DAC_HAS_INFO
    -DAC_HAS_WARNING
    -DAC_HAS_ERROR
    -DAC_HAS_CRITICAL
    -DAC_HAS_DEBUG
    -DTIXML_USE_STL
    -DLINUX_DAEMON
    -DENCRYPT
    -std=c++11)

include_directories(
    ./
    ./redis
    ./business
    ${BASE_DIR}
    ${BASE_DIR}/pbgen
    ${BASE_DIR}/common
    ${BASE_DIR}/http
    ${BASE_DIR}/impdu
    ${BASE_DIR}/jsoncpp
    ${BASE_DIR}/netlib
    ${BASE_DIR}/threadpool
    ${THIRD_DIR}/slog/include
    ${THIRD_DIR}/protobuf/include
    ${THIRD_DIR}/libsecurity/include
    /usr/include/mysql)

link_directories(
    ${PB_LIB_DIR}
    ${SLOG_LIB_DIR}
    ${BASE_LIB_DIR}
    ${MYSQL_LIB_DIR})

add_executable(db_proxy_server
    ${SRC_LIST1}
    ${SRC_LIST2}
    ${SRC_LIST3})

target_link_libraries(db_proxy_server pthread base protobuf-lite mysqlclient hiredis curl slog crypto)

