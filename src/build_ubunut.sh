#!/bin/bash

pack_folder() {
    CURPWD=$PWD

    echo $CURPWD
    ###################################################################################
    # 基本工具安装
    # apt-get -y install cmake
    # apt-get -y install libuu-dev
    # apt-get -y install libcurl4-openssl-dev
    # apt-get -y install libssl-dev
    # apt-get -y install libcurl-dev

    ###################################################################################
    # step1.第三方及本地库编译
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

    rm -f base/version.h
    echo "#ifndef __VERSION_H__" > base/version.h
    echo "#define __VERSION_H__" >> base/version.h
    echo "#define VERSION \"$1\"" >> base/version.h
    echo "#endif" >> base/version.h

    cd $CURPWD
    for i in base slog; do
        cd $CURPWD/$i

        if [ ! -d build ]
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
    if [ ! -d lib ]
    then
        mkdir lib
    fi
    # copy libbase.a
    cp base/build/libbase.a lib/
    # copy libslog.so
    mkdir -p base/slog/lib
    cp slog/slog_api.h base/slog/
    cp slog/build/libslog.so base/slog/lib/

    ###################################################################################
    # step2.所有业务服务器编译
    # login_server msg_server route_server
    # msfs file_server db_proxy_server http_msg_server
    # push_server
    cd $CURPWD
    for i in route_server msg_server http_msg_server file_server push_server db_proxy_server msfs login_server; do
        cd $CURPWD/$i

        if [ ! -d build ]
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

    ###################################################################################
    # step3.打包所有编译结果与配置文件
    # copy executables and configs to im-server-$version

    pack_folder=pack_folder
    target_name=im-server-$1.tar.gz
    if [ -e "$target_name" ]; then
        rm $target_name
    fi

    rm -rf pack_folder
    mkdir -p ../$pack_folder
    mkdir -p ../$pack_folder/login_server
    mkdir -p ../$pack_folder/route_server
    mkdir -p ../$pack_folder/msg_server
    mkdir -p ../$pack_folder/file_server
    mkdir -p ../$pack_folder/msfs
    mkdir -p ../$pack_folder/http_msg_server
    mkdir -p ../$pack_folder/push_server
    mkdir -p ../$pack_folder/db_proxy_server
    mkdir -p ../$pack_folder/lib
    mkdir -p ../$pack_folder/conf

    # copy executables
    cp login_server/build/login_server ../$pack_folder/login_server/
    cp route_server/build/route_server ../$pack_folder/route_server/
    cp msg_server/build/msg_server ../$pack_folder/msg_server/
    cp http_msg_server/build/http_msg_server ../$pack_folder/http_msg_server/
    cp file_server/build/file_server ../$pack_folder/file_server/
    cp push_server/build/push_server ../$pack_folder/push_server/
    cp db_proxy_server/build/db_proxy_server ../$pack_folder/db_proxy_server/
    cp msfs/build/msfs ../$pack_folder/msfs/

    # copy conf
    cp login_server/loginserver.conf ../$pack_folder/conf/
    cp route_server/routeserver.conf ../$pack_folder/conf/
    cp msg_server/msgserver.conf ../$pack_folder/conf/
    cp http_msg_server/httpmsgserver.conf ../$pack_folder/conf/
    cp file_server/fileserver.conf ../$pack_folder/conf/
    cp push_server/pushserver.conf ../$pack_folder/conf/
    cp db_proxy_server/dbproxyserver.conf ../$pack_folder/conf/
    cp msfs/msfs.conf ../$pack_folder/conf/
    cp slog/log4cxx.properties ../$pack_folder/conf/

    # copy libs
    cp slog/build/libslog.so  ../$pack_folder/lib/
    cp -a protobuf/lib/libprotobuf-lite.so* ../$pack_folder/lib/

    # copy sript
    cp tools/restart.sh ../$pack_folder/
    cp tools/monitor.sh ../$pack_folder/
    cp tools/ttopen.sh ../$pack_folder/
    cp tools/setup.sh ../$pack_folder/

    # copy sql
    cp tools/ttopen.sql ../$pack_folder/

    # copy daeml
    cp tools/daeml ../$pack_folder/

    cd ../
    
    tar zcvf $target_name $pack_folder
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
    cd ../
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
    rm -rf build

    # login_server
    cd ../login_server
    echo "clean login_server"
    rm -rf build
    
    # msg_server
    cd ../msg_server
    echo "clean msg_server"
    rm -rf build

    # http_msg_server
    cd ../http_msg_server
    echo "clean http_msg_server"
    rm -rf build

    # file_server
    cd ../file_server
    echo "clean file_server"
    rm -rf build

    # push_server
    cd ../push_server
    echo "clean push_server"
    rm -rf build

    # db_proxy_server
    cd ../db_proxy_server
    echo "clean db_proxy_server"
    rm -rf build

    # msfs
    cd ../msfs
    echo "clean msfs"
    rm -rf build
}

print_help() {
    echo "Usage: "
    echo "  $0 clean --- clean all build"
    echo "  $0 distclean --- clean all build and cmakecache"
    echo "  $0 version version_str --- build a version"
}

build() {
    CURPWD=$PWD
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

    if [ ! -d $1 ]
    then
        echo "build failed: folder[$1] not exist."
        exit;
    fi

    cd $1
    rm -rf build
    mkdir build
    cd build
    cmake ../
    make
    if [ $? -eq 0 ]; then
        echo ">>> module build: $1 succeed~";
    else
        echo ">>> module build: $1 failed!";
        exit;
    fi

    cd $CURPWD
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
        clean
        echo "version $2 in building..."
        pack_folder $2
        ;;
    login_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    msg_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    route_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    http_msg_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    file_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    push_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    db_proxy_server)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    msfs)
        build $1
        cp $1/build/$1 ../im-server-1.0/$1
        ;;
    base)
        build $1
        cp base/build/libbase.a lib/
        ;;
    *)
    print_help
    ;; 
esac



