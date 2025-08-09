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

    # 环境变量设置
    # base
    export CPLUS_INCLUDE_PATH=$PWD/base:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$PWD/base/bin:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$PWD/base/bin:$LIBRARY_PATH
    # slog
    export CPLUS_INCLUDE_PATH=$PWD/third/slog/include:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$PWD/third/slog/lib:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$PWD/third/slog/lib:$LIBRARY_PATH
    # protobuf
    export CPLUS_INCLUDE_PATH=$PWD/third//protobuf/include:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$PWD/third//protobuf/lib:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$PWD/third//protobuf/lib:$LIBRARY_PATH
    ###################################################################################
    # step1.第三方及本地库编译
    rm -f base/version.h
    echo "#ifndef __VERSION_H__" > base/version.h
    echo "#define __VERSION_H__" >> base/version.h
    echo "#define VERSION \"$1\"" >> base/version.h
    echo "#endif" >> base/version.h

    ###################################################################################
    # step2.所有业务服务器编译
    # login_server msg_server route_server
    # msfs file_server db_proxy_server http_msg_server
    # push_server
    cd $CURPWD
    for i in base route_server msg_server http_msg_server file_server push_server db_proxy_server msfs login_server; do
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
    cp login_server/bin/login_server ../$pack_folder/login_server/
    cp route_server/bin/route_server ../$pack_folder/route_server/
    cp msg_server/bin/msg_server ../$pack_folder/msg_server/
    cp http_msg_server/bin/http_msg_server ../$pack_folder/http_msg_server/
    cp file_server/bin/file_server ../$pack_folder/file_server/
    cp push_server/bin/push_server ../$pack_folder/push_server/
    cp db_proxy_server/bin/db_proxy_server ../$pack_folder/db_proxy_server/
    cp msfs/bin/msfs ../$pack_folder/msfs/

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
    cp third/slog/lib/libslog.so  ../$pack_folder/lib/
    cp -a third/protobuf/lib/libprotobuf-lite.so* ../$pack_folder/lib/

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
    rm -rf build bin
    cd ../login_server
    rm -rf build bin
    cd ../route_server
    rm -rf build bin
    cd ../msg_server
    rm -rf build bin
    cd ../http_msg_server
    rm -rf build bin
    cd ../file_server
    rm -rf build bin
    cd ../push_server
    rm -rf build bin
    cd ../db_proxy_server
    rm -rf build bin
    cd ../push_server
    rm -rf build bin
    cd ../
}

print_help() {
    echo "Usage: "
    echo "  $0 clean --- clean all build"
    echo "  $0 distclean --- clean all build and cmakecache"
    echo "  $0 version version_str --- build a version"
}

build() {
    CURPWD=$PWD
    if [ ! -d $1 ]
    then
        echo "build failed: folder[$1] not exist."
        exit;
    fi

    cd $1
    rm -rf build bin
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

    # 同步到打包目录下
    # cp $1/bin/$1 ../im-server-1.0/$1
}

case $1 in
    clean)
        echo "clean all build..."
        clean
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
    login_server|msg_server|route_server|http_msg_server|file_server|push_server|db_proxy_server|msfs|base)
        build "$1"
        ;;
    *)
        print_help
        ;;
esac

