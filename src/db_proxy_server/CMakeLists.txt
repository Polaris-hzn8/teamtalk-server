cmake_minimum_required(VERSION 2.6)
project(db_proxy_server)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST1)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/redis SRC_LIST2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/business SRC_LIST3)

set(EXECUTABLE_OUTPUT_PATH ./)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# 编译链接的库文件
if(APPLE)
    set(PB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../base/pb/lib/mac")
elseif(UNIX)
    set(PB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../base/pb/lib/linux")
endif()
set(SLOG_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../base/slog/lib")
set(BASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(MYSQL_LIB /usr/lib64/mysql)

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
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/redis
    ${CMAKE_CURRENT_SOURCE_DIR}/business
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/pb/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/http
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/impdu
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/jsoncpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/netlib
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/security
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/slog
    ${CMAKE_CURRENT_SOURCE_DIR}/../base/threadpool
    /usr/include/mysql)

link_directories(
    ${PB_LIB}
    ${BASE_LIB}
    ${MYSQL_LIB}
    ${SLOG_LIB})

add_executable(db_proxy_server
    ${SRC_LIST1}
    ${SRC_LIST2}
    ${SRC_LIST3})

target_link_libraries(db_proxy_server pthread base protobuf-lite mysqlclient hiredis curl slog crypto)

