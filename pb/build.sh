#!/bin/sh
SRC_DIR=./
DST_DIR=./gen
CPP_DIR=../src/base/pb/protocol
PROTOC=$PWD/../src/protobuf/bin/protoc

build() {
    echo "building..."
    #C++
    mkdir -p $DST_DIR/cpp
    $PROTOC -I=$SRC_DIR --cpp_out=$DST_DIR/cpp/ $SRC_DIR/*.proto

    #JAVA
    mkdir -p $DST_DIR/java
    $PROTOC -I=$SRC_DIR --java_out=$DST_DIR/java/ $SRC_DIR/*.proto

    #PYTHON
    mkdir -p $DST_DIR/python
    $PROTOC -I=$SRC_DIR --python_out=$DST_DIR/python/ $SRC_DIR/*.proto

    echo "build finished."
}

sync() {
    echo "syncing..."
    #C++
    sudo cp $DST_DIR/cpp/* $CPP_DIR/

    echo "sync finished."
    rm -rf ./gen
}

print_help() {
    echo "Usage: "
	echo "  $0 build --- build protos."
	echo "  $0 sync --- sync output files."
}

case $1 in
	build)
        build
		;;
    sync)
        sync
        ;;
    help)
        print_help
        ;;
	*)
		build
        sync
		;;
esac
