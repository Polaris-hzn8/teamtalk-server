cmake_minimum_required(VERSION 2.6)
project(msfs)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base)
set(THIRD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

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
    -DTIXML_USE_STL
    -DLINUX_DAEMON
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
    ${SLOG_LIB_DIR}
    ${BASE_LIB_DIR})

add_executable(msfs ${SRC_LIST})

target_link_libraries(msfs base slog pthread)
 
