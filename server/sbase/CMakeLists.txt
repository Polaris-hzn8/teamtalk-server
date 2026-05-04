cmake_minimum_required(VERSION 3.10)
project(teamtalk-sbase)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDK_DIR ${CMAKE_SOURCE_DIR}/../../../.sdk)
set(CMAKE_INSTALL_PREFIX "${SDK_DIR}/teamtalk-sbase" CACHE PATH "Install directory" FORCE)

file(GLOB_RECURSE PROJECT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

message(STATUS "Configuring for Linux")
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
add_compile_options(-g -W -Wall)

include_directories(
    "${CMAKE_SOURCE_DIR}"
    ${SDK_DIR}/teamtalk-imcore/include
    ${SDK_DIR}/protobuf/include
)

link_directories(
    ${SDK_DIR}/teamtalk-imcore/lib
    ${SDK_DIR}/protobuf/lib
)

add_library(teamtalk-sbase SHARED ${PROJECT_SOURCES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_link_libraries(teamtalk-sbase
    teamtalk-imcore
    protobuf-lite
)

install(TARGETS teamtalk-sbase
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/" DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)
