cmake_minimum_required(VERSION 2.6)
project(slog)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(LOG4XX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../log4cxx)

include_directories(include ${LOG4XX_DIR}/include)

aux_source_directory(./ SRC_LIST)

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

link_directories(${LOG4XX_DIR}/lib)

add_library(slog SHARED ${SRC_LIST})

target_link_libraries(slog log4cxx)
