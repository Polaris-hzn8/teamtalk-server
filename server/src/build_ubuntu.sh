#!/bin/bash

build() {
    # 基本工具安装
    # apt-get -y install cmake
    # apt-get -y install libuu-dev
    # apt-get -y install libcurl4-openssl-dev
    # apt-get -y install libssl-dev
    # apt-get -y install libcurl-dev

    # 第三方及本地库引入
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

    echo "#ifndef __VERSION_H__" > base/version.h
    echo "#define __VERSION_H__" >> base/version.h
    echo "#define VERSION \"$1\"" >> base/version.h
    echo "#endif" >> base/version.h

    if [ ! -d lib ]
    then
        mkdir lib
    fi

    CURPWD=$PWD

    # START COMPILE SERVERS
    # base slog tools
    # route_server msg_server http_msg_server file_server push_server db_proxy_server msfs login_server
    for i in route_server msg_server http_msg_server file_server push_server db_proxy_server msfs login_server; do
        cd $CURPWD/$i

        if [ ! -d lib ]
        then
            mkdir build
        fi

        cd build
        cmake ../
        make
        if [ $? -eq 0 ]; then
            echo ">>> makeout: $i succeed~";
        else
            echo ">>> makeout: $i failed!";
            exit;
        fi
    done

    cd $CURPWD

    # copy libbase.a
    cp base/build/libbase.a lib/
    # copy libslog.so
    mkdir base/slog/lib
    cp slog/slog_api.h base/slog/
    cp slog/build/libslog.so base/slog/lib/

    # copy executables to run/
    mkdir -p ../run/login_server
    mkdir -p ../run/route_server
    mkdir -p ../run/msg_server
    mkdir -p ../run/file_server
    mkdir -p ../run/msfs
    mkdir -p ../run/push_server
    mkdir -p ../run/http_msg_server
    mkdir -p ../run/db_proxy_server
    cp login_server/build/login_server ../run/login_server/
    cp route_server/build/route_server ../run/route_server/
    cp msg_server/build/msg_server ../run/msg_server/
    cp http_msg_server/build/http_msg_server ../run/http_msg_server/
    cp file_server/build/file_server ../run/file_server/
    cp push_server/build/push_server ../run/push_server/
    cp db_proxy_server/build/db_proxy_server ../run/db_proxy_server/
    cp msfs/build/msfs ../run/msfs/
    cp tools/daeml ../run/

    # 软件打包
    # im-server-1.0
    # copy executables and configs to im-server-1.0/
    build_version=im-server-$1
    build_name=$build_version.tar.gz
    if [ -e "$build_name" ]; then
        rm $build_name
    fi

    mkdir -p ../$build_version
    mkdir -p ../$build_version/login_server
    mkdir -p ../$build_version/route_server
    mkdir -p ../$build_version/msg_server
    mkdir -p ../$build_version/file_server
    mkdir -p ../$build_version/msfs
    mkdir -p ../$build_version/http_msg_server
    mkdir -p ../$build_version/push_server
    mkdir -p ../$build_version/db_proxy_server
    mkdir -p ../$build_version/lib

    cp login_server/build/login_server ../$build_version/login_server/
    cp route_server/build/route_server ../$build_version/route_server/
    cp msg_server/build/msg_server ../$build_version/msg_server/
    cp http_msg_server/build/http_msg_server ../$build_version/http_msg_server/
    cp file_server/build/file_server ../$build_version/file_server/
    cp push_server/build/push_server ../$build_version/push_server/
    cp db_proxy_server/build/db_proxy_server ../$build_version/db_proxy_server/
    cp msfs/build/msfs ../$build_version/msfs/

    cp login_server/loginserver.conf ../$build_version/login_server/
    cp route_server/routeserver.conf ../$build_version/route_server/
    cp msg_server/msgserver.conf ../$build_version/msg_server/
    cp http_msg_server/httpmsgserver.conf ../$build_version/http_msg_server/
    cp file_server/fileserver.conf ../$build_version/file_server/
    cp push_server/pushserver.conf ../$build_version/push_server/
    cp db_proxy_server/dbproxyserver.conf ../$build_version/db_proxy_server/
    cp msfs/msfs.conf ../$build_version/msfs/

    cp slog/log4cxx.properties ../$build_version/lib/
    cp -a slog/lib/liblog4cxx.so* ../$build_version/lib/
    cp slog/libslog.so  ../$build_version/lib/

    cp ../run/restart.sh ../$build_version/
    cp sync_lib_for_zip.sh ../$build_version/
    cp tools/daeml ../$build_version/

    cd ../
    
    tar zcvf    $build_name $build_version

    rm -rf $build_version
}

clean() {
    cd base
    rm -rf build
    cd ../login_server
    rm -rf build
    cd ../route_server
    rm -rf build
    cd ../msg_server
    rm -rf build
    cd ../http_msg_server
    rm -rf build
    cd ../file_server
    rm -rf build
    cd ../push_server
    rm -rf build
    cd ../db_proxy_server
    rm -rf build
    cd ../push_server
    rm -rf build
}

distclean() {
    # tools
    # base
    cd base
    echo "clean base"
    rm -rf build pb/google pb/lib slog

    # slog
    cd ../slog
    echo "clean slog"
    rm -rf build include lib

    # protobuf
    cd ../protobuf
    echo "clean protobuf"
    rm -rf bin include lib protobuf-2.6.1

    # log4cxx
    echo "clean log4cxx"
    cd ../log4cxx
    rm -rf apache-log4cxx-0.10.0 include lib

    # route_server
    cd ../route_server
    echo "clean route_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake route_server

    # login_server
    cd ../login_server
    echo "clean login_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake login_server
    
    # msg_server
    cd ../msg_server
    echo "clean msg_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake msg_server

    # http_msg_server
    cd ../http_msg_server
    echo "clean http_msg_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake http_msg_server

    # file_server
    cd ../file_server
    echo "clean file_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake file_server

    # push_server
    cd ../push_server
    echo "clean push_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake push_server

    # db_proxy_server
    cd ../db_proxy_server
    echo "clean db_proxy_server"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake db_proxy_server

    # msfs
    cd ../msfs
    echo "clean msfs"
    make clean
    rm -rf Makefile CMakeCache.txt CMakeFiles cmake_install.cmake msfs
}

print_help() {
    echo "Usage: "
    echo "  $0 clean --- clean all build"
    echo "  $0 distclean --- clean all build and cmakecache"
    echo "  $0 version version_str --- build a version"
}

case $1 in
    clean)
        echo "clean all build..."
        clean
        ;;
    distclean)
        echo "clean all build and cmakecache..."
        distclean
        ;;
    version)
        if [ $# != 2 ]; then 
            echo $#
            print_help
            exit
        fi

        echo $2
        echo "build..."
        build $2
        ;;
    *)
    print_help
    ;;
esac



