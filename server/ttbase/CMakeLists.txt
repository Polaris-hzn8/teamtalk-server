cmake_minimum_required(VERSION 2.6)
project(ttbase)

aux_source_directory("${CMAKE_SOURCE_DIR}/common" SRC_COMMON)
aux_source_directory("${CMAKE_SOURCE_DIR}/config_reader" SRC_CONFIG_READER)
aux_source_directory("${CMAKE_SOURCE_DIR}/enc" SRC_ENC)
aux_source_directory("${CMAKE_SOURCE_DIR}/http" SRC_HTTP)
aux_source_directory("${CMAKE_SOURCE_DIR}/jsoncpp" SRC_JSONCPP)
aux_source_directory("${CMAKE_SOURCE_DIR}/netlib" SRC_NETLIB)
aux_source_directory("${CMAKE_SOURCE_DIR}/netlib/core" SRC_NETLIB_CORE)
aux_source_directory("${CMAKE_SOURCE_DIR}/netlib/impdu" SRC_NETLIB_IMPDU)
aux_source_directory("${CMAKE_SOURCE_DIR}/netlib/utility" SRC_NETLIB_UTILITY)
aux_source_directory("${CMAKE_SOURCE_DIR}/slog" SRC_SLOG)
aux_source_directory("${CMAKE_SOURCE_DIR}/string" SRC_STRING)
aux_source_directory("${CMAKE_SOURCE_DIR}/threadpool" SRC_THREADPOOL)
aux_source_directory("${CMAKE_SOURCE_DIR}/token" SRC_TOKEN)
aux_source_directory("${CMAKE_SOURCE_DIR}/ttpbdef" SRC_TPBDEF)

# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/rootfs/sbin")
# 动态库输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/rootfs/lib")
# 静态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/rootfs/lib")

set(THIRD_DIR "${CMAKE_SOURCE_DIR}/../../third")

if(APPLE)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
elseif(UNIX)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
endif()

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
    -DLINUX_DAEMON)

include_directories(
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/common"
    "${CMAKE_SOURCE_DIR}/config_reader"
    "${CMAKE_SOURCE_DIR}/enc"
    "${CMAKE_SOURCE_DIR}/http"
    "${CMAKE_SOURCE_DIR}/jsoncpp"
    "${CMAKE_SOURCE_DIR}/netlib"
    "${CMAKE_SOURCE_DIR}/netlib/core"
    "${CMAKE_SOURCE_DIR}/netlib/impdu"
    "${CMAKE_SOURCE_DIR}/netlib/utility"
    "${CMAKE_SOURCE_DIR}/slog"
    "${CMAKE_SOURCE_DIR}/string"
    "${CMAKE_SOURCE_DIR}/threadpool"
    "${CMAKE_SOURCE_DIR}/token"
    "${CMAKE_SOURCE_DIR}/ttpbdef"
    ${THIRD_DIR}/protobuf/include)

link_directories(
    ${PB_LIB_DIR})

add_library(ttbase SHARED
    ${SRC_COMMON}
    ${SRC_CONFIG_READER}
    ${SRC_ENC}
    ${SRC_HTTP}
    ${SRC_JSONCPP}
    ${SRC_NETLIB}
    ${SRC_NETLIB_CORE}
    ${SRC_NETLIB_IMPDU}
    ${SRC_NETLIB_UTILITY}
    ${SRC_SLOG}
    ${SRC_STRING}
    ${SRC_THREADPOOL}
    ${SRC_TOKEN}
    ${SRC_TPBDEF})

target_link_libraries(ttbase protobuf pthread)
