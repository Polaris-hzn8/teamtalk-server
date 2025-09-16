#!/bin/sh
SRC_DIR=./
GEM_DIR=./gen
DEST_DIR=../src/base/pbgen
PROTOC=$PWD/../src/third/protobuf/bin/protoc

build() {
    echo "building..."
    #C++
    mkdir -p $GEM_DIR/cpp
    $PROTOC -I=$SRC_DIR --cpp_out=$GEM_DIR/cpp/ $SRC_DIR/*.proto

    #JAVA
    mkdir -p $GEM_DIR/java
    $PROTOC -I=$SRC_DIR --java_out=$GEM_DIR/java/ $SRC_DIR/*.proto

    #PYTHON
    mkdir -p $GEM_DIR/python
    $PROTOC -I=$SRC_DIR --python_out=$GEM_DIR/python/ $SRC_DIR/*.proto

    echo "build finished."
}

sync() {
    echo "syncing..."
    #C++
    sudo cp $GEM_DIR/cpp/* $DEST_DIR/

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
