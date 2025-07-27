#!/bin/bash

FILE_SERVER=file_server
LOGIN_SERVER=login_server
MSG_SERVER=msg_server
ROUTE_SERVER=route_server
HTTP_MSG_SERVER=http_msg_server
PUSH_SERVER=push_server
DB_PROXY_SERVER=db_proxy_server
MSFS_SERVER=msfs

FILE_SERVER_CONF=fileserver.conf
LOGIN_SERVER_CONF=loginserver.conf
MSG_SERVER_CONF=msgserver.conf
ROUTE_SERVER_CONF=routeserver.conf
MSFS_SERVER_CONF=msfs.conf
HTTP_MSG_SERVER_CONF=httpmsgserver.conf
PUSH_SERVER_CONF=pushserver.conf
DB_PROXY_SERVER_CONF=dbproxyserver.conf

print_hello(){
    echo "==========================================="
    echo "$1 im server for TeamTalk"
    echo "==========================================="
}

check_user() {
    if [ $(id -u) != "0" ]; then
        echo "Error: You must be root to run this script, please use root to install im_c++_server"
        exit 1
    fi
}

check_os() {
    OS_VERSION=$(less /etc/issue)
    OS_BIT=$(getconf LONG_BIT)
    echo "$OS_VERSION, $OS_BIT bit..." 
    return 0
    if [[ $OS_VERSION =~ "CentOS" ]]; then
        if [ $OS_BIT == 64 ]; then
            return 0
        else
            echo "Error: OS must be CentOS 64bit to run this script."
            exit 1
        fi
    else
        echo "Error: OS must be CentOS 64bit to run this script."
        exit 1
    fi
}

clean_yum() {
    YUM_PID=/var/run/yum.pid
    if [ -f "$YUM_PID" ]; then
        set -x
        rm -f YUM_PID
        killall yum
        set +x
    fi
}

remove_lib() {
    rm -rf ./$LOGIN_SERVER_CONF/lib*
    rm -rf ./$MSG_SERVER/lib*
    rm -rf ./$ROUTE_SERVER/lib*
    rm -rf ./$FILE_SERVER/lib*
    rm -rf ./$MSFS_SERVER/lib*
    rm -rf ./$HTTP_MSG_SERVER/lib*
    rm -rf ./$PUSH_SERVER/lib*
    rm -rf ./$DB_PROXY_SERVER/lib*
}

sync_lib() {
    cp -a ./lib/libslog.so  ./login_server/
    cp -a ./lib/libprotobuf-lite* ./login_server/

    cp -a ./lib/libslog.so  ./route_server/
    cp -a ./lib/libprotobuf-lite* ./route_server/

    cp -a ./lib/libslog.so  ./msg_server/
    cp -a ./lib/libprotobuf-lite* ./msg_server/

    cp -a ./lib/libslog.so  ./http_msg_server/
    cp -a ./lib/libprotobuf-lite* ./http_msg_server/

    cp -a ./lib/libslog.so  ./file_server/
    cp -a ./lib/libprotobuf-lite* ./file_server/

    cp -a ./lib/libslog.so  ./push_server/
    cp -a ./lib/libprotobuf-lite* ./push_server/

    cp -a ./lib/libslog.so  ./db_proxy_server/
    cp -a ./lib/libprotobuf-lite* ./db_proxy_server/

    cp -a ./lib/libslog.so  ./msfs/
    cp -a ./lib/libprotobuf-lite* ./msfs/
}

sync_conf() {
    # slog conf
    cp -a ./conf/log4cxx.properties ./login_server/
    cp -a ./conf/log4cxx.properties ./route_server/
    cp -a ./conf/log4cxx.properties ./msg_server/
    cp -a ./conf/log4cxx.properties ./http_msg_server/
    cp -a ./conf/log4cxx.properties ./file_server/
    cp -a ./conf/log4cxx.properties ./push_server/
    cp -a ./conf/log4cxx.properties ./db_proxy_server/
    cp -a ./conf/log4cxx.properties ./msfs/
    # server conf
    cp -f ./conf/$LOGIN_SERVER_CONF $LOGIN_SERVER/
    cp -f ./conf/$MSG_SERVER_CONF $MSG_SERVER/
    cp -f ./conf/$ROUTE_SERVER_CONF $ROUTE_SERVER/
    cp -f ./conf/$FILE_SERVER_CONF $FILE_SERVER/
    cp -f ./conf/$MSFS_SERVER_CONF $MSFS_SERVER/
    cp -f ./conf/$HTTP_MSG_SERVER_CONF $HTTP_MSG_SERVER/
    cp -f ./conf/$PUSH_SERVER_CONF	$PUSH_SERVER/
    cp -f ./conf/$DB_PROXY_SERVER_CONF $DB_PROXY_SERVER/
}

run_im_server() {
    ./restart.sh $LOGIN_SERVER
    ./restart.sh $ROUTE_SERVER
    ./restart.sh $MSG_SERVER
    ./restart.sh $FILE_SERVER
    ./restart.sh $MSFS_SERVER
    ./restart.sh $HTTP_MSG_SERVER
    ./restart.sh $PUSH_SERVER
    ./restart.sh $DB_PROXY_SERVER
}

print_help() {
    echo "Usage: "
    echo "  $0 check --- check environment"
    echo "  $0 install --- check & run scripts to install"
}

case $1 in
    check)
        print_hello $1
        check_user
        ;;
    install)
        print_hello $1
        check_user

        chmod +x ./daeml
        chmod 755 ./restart.sh
        chmod 755 ./monitor.sh
        chmod 755 ./ttopen.sh

        sync_lib
        sync_conf
        run_im_server
        ;;
    restart)
        print_hello $1
        check_user

        sync_lib
        sync_conf
        run_im_server
        ;;
    remove_lib)
        remove_lib
        ;;
    *)
        print_help
        ;;
esac



