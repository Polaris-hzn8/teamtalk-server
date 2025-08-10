cmake_minimum_required(VERSION 2.6)
project(base)

aux_source_directory(./pbgen SRC_LIST1)
aux_source_directory(./jsoncpp SRC_LIST2)
aux_source_directory(./http SRC_LIST3)
aux_source_directory(./impdu SRC_LIST4)
aux_source_directory(./common SRC_LIST5)
aux_source_directory(./netlib SRC_LIST6)
aux_source_directory(./threadpool SRC_LIST7)

set(THIRD_DIR ../third)

if(APPLE)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
elseif(UNIX)
    set(PB_LIB_DIR ${THIRD_DIR}/protobuf/lib)
endif()
set(SLOG_LIB_DIR ${THIRD_DIR}/slog/lib)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

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
    ./
    ./common
    ./http
    ./impdu
    ./jsoncpp
    ./netlib
    ./pbgen
    ./threadpool
    ${THIRD_DIR}/slog/include
    ${THIRD_DIR}/protobuf/include)

link_directories(
    ${SLOG_LIB_DIR}
    ${PB_LIB_DIR})

add_library(base STATIC
    ${SRC_LIST1}
    ${SRC_LIST2}
    ${SRC_LIST3}
    ${SRC_LIST4}
    ${SRC_LIST5}
    ${SRC_LIST6}
    ${SRC_LIST7})

target_link_libraries(base slog crypto pthread)
