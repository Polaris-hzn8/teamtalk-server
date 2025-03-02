#!/bin/bash

# base
export CPLUS_INCLUDE_PATH=$PWD/base:$CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH=$PWD/base/build:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/base/build:$LIBRARY_PATH
# slog
export CPLUS_INCLUDE_PATH=$PWD/slog/include:$CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH=$PWD/slog/build:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/slog/build:$LIBRARY_PATH
# protobuf
export CPLUS_INCLUDE_PATH=$PWD/protobuf/include:$CPLUS_INCLUDE_PATH
export LD_LIBRARY_PATH=$PWD/protobuf/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/protobuf/lib:$LIBRARY_PATH

cd db_proxy_server
rm -rf build
mkdir build
cd build
cmake ../
make

