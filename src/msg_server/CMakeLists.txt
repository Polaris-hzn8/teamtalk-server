cmake_minimum_required(VERSION 2.6)
project(msgserver)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../base)
set(THIRD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if(APPLE)
    set(SECURITY_LIB_DIR ${THIRD_DIR}/libsecurity/lib/mac)
elseif(UNIX)
    set(SECURITY_LIB_DIR ${THIRD_DIR}/libsecurity/lib/unix)
endif()
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
    ${BASE_DIR}
    ${BASE_DIR}/common
    ${BASE_DIR}/http
    ${BASE_DIR}/jsoncpp
    ${BASE_DIR}/netlib
    ${BASE_DIR}/netlib/impdu
    ${BASE_DIR}/netlib/pbgen
    ${BASE_DIR}/threadpool
    ${THIRD_DIR}/slog/include
    ${THIRD_DIR}/protobuf/include
    ${THIRD_DIR}/libsecurity/include)

link_directories(
    ${PB_LIB_DIR}
    ${SLOG_LIB_DIR}
    ${BASE_LIB_DIR}
    ${SECURITY_LIB_DIR})

add_executable(msg_server ${SRC_LIST})

target_link_libraries(msg_server base slog protobuf-lite crypto pthread)
 
# rpath动态库查找
set_target_properties(msg_server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
