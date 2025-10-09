cmake_minimum_required(VERSION 2.6)
project(http_msg_server)

aux_source_directory(./ SRC_LIST)

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base)
set(THIRD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
set(SLOG_LIB_DIR ${THIRD_DIR}/slog/lib)
set(BASE_LIB_DIR ${BASE_DIR}/bin)

add_definitions(
    -g -W -Wall
    -D_REENTRANT
    -D_FILE_OFFSET_BITS=64
    -DAC_HAS_INFO
    -DAC_HAS_WARNING
    -DAC_HAS_ERROR
    -DAC_HAS_CRITICAL
    -DAC_HAS_DEBUG
    -DLINUX_DAEMON
    -DTIXML_USE_STL
    -std=c++11)

include_directories(
    ./
    ${BASE_DIR}
    ${BASE_DIR}/pbgen
    ${BASE_DIR}/common
    ${BASE_DIR}/http
    ${BASE_DIR}/impdu
    ${BASE_DIR}/jsoncpp
    ${BASE_DIR}/netlib
    ${BASE_DIR}/threadpool
    ${THIRD_DIR}/slog/include
    ${THIRD_DIR}/protobuf/include)

link_directories(
    ${PB_LIB_DIR}
    ${SLOG_LIB_DIR}
    ${BASE_LIB_DIR})

add_executable(http_msg_server ${SRC_LIST})

target_link_libraries(http_msg_server slog base protobuf-lite pthread)

# rpath动态库查找
set_target_properties(http_msg_server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
