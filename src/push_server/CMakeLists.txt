cmake_minimum_required(VERSION 2.6)
project(pushserver)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST1)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/bufferloop SRC_LIST2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/socket SRC_LIST3)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/thread SRC_LIST4)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/timer SRC_LIST5)

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
    -DTIXML_USE_STL
    -DLINUX_DAEMON
    -std=c++11)

include_directories(
    ./
    ${BASE_DIR}/pbgen
    ${THIRD_DIR}/slog/include
    ${THIRD_DIR}/protobuf/include)

link_directories(
    ${PB_LIB_DIR}
    ${SLOG_LIB_DIR}
    ${BASE_LIB_DIR})

add_executable(push_server ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4} ${SRC_LIST5})

target_link_libraries(push_server base slog pthread protobuf-lite ssl)

# rpath动态库查找
set_target_properties(push_server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
