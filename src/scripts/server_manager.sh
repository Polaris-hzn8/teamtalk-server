#!/bin/bash

# Teamtalk Server Management Script
# 服务器进程组管理脚本

## 服务器配置变量
readonly FILE_SERVER=file_server
readonly LOGIN_SERVER=login_server
readonly MSG_SERVER=msg_server
readonly ROUTE_SERVER=route_server
readonly HTTP_MSG_SERVER=http_msg_server
readonly PUSH_SERVER=push_server
readonly DB_PROXY_SERVER=db_proxy_server
readonly MSFS_SERVER=msfs

readonly FILE_SERVER_CONF=fileserver.conf
readonly LOGIN_SERVER_CONF=loginserver.conf
readonly MSG_SERVER_CONF=msgserver.conf
readonly ROUTE_SERVER_CONF=routeserver.conf
readonly HTTP_MSG_SERVER_CONF=httpmsgserver.conf
readonly PUSH_SERVER_CONF=pushserver.conf
readonly DB_PROXY_SERVER_CONF=dbproxyserver.conf
readonly MSFS_SERVER_CONF=msfs.conf

# 所有服务器列表
readonly ALL_SERVERS=(
    "$FILE_SERVER"
    "$LOGIN_SERVER"
    "$MSG_SERVER"
    "$ROUTE_SERVER" 
    "$HTTP_MSG_SERVER"
    "$PUSH_SERVER"
    "$DB_PROXY_SERVER"
    "$MSFS_SERVER"
)

## 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_hello(){
    echo "==========================================="
    echo "$1 im server for TeamTalk"
    echo "==========================================="
}

######################################################################
# 运行校验
check_user() {
    if [ $(id -u) != "0" ]; then
        print_color "$RED" "Error: You must be root to run this script"
        exit 1
    fi
}

check_environment() {
    # 获取操作系统版本信息
    OS_VERSION=$(less /etc/issue 2>/dev/null || echo "Unknown")
    # 获取系统位数（32位或64位）
    OS_BIT=$(getconf LONG_BIT)
    # 用蓝色显示系统信息
    print_color "$BLUE" "OS: $OS_VERSION, $OS_BIT bit"
    # 检查必要的目录和文件
    if [ ! -d "./lib" ] || [ ! -d "./conf" ]; then
        print_color "$RED" "Error: lib or conf directory not found"
        exit 1
    fi
}

######################################################################
# 服务运行管理
## 批量操作函数
manage_all_servers() {
    local action=$1
    local failed_servers=()
    
    for server in "${ALL_SERVERS[@]}"; do
        print_color "$BLUE" "=== $action $server ==="
        if ! ${action}_server "$server"; then
            failed_servers+=("$server")
        fi
        echo
    done
    
    if [ ${#failed_servers[@]} -gt 0 ]; then
        print_color "$YELLOW" "Warning: The following servers had issues: ${failed_servers[*]}"
    fi
}

# 重启服务
restart_server() {
    local server=$1
    local server_dir="./$server"
    
    if [ ! -d "$server_dir" ]; then
        print_color "$RED" "Error: Server directory $server_dir not found"
        return 1
    fi
    
    cd "$server_dir" || return 1
    
    # 检查配置文件
    if [ ! -e *.conf ]; then
        print_color "$YELLOW" "Warning: No config file found for $server"
        cd ..
        return 1
    fi
    
    # 特殊处理 msfs 目录
    if [ "$server" = "msfs" ] && [ ! -d "tmp" ]; then
        mkdir -p tmp
        print_color "$GREEN" "Created msfs/tmp directory"
    fi
    
    # 停止运行中的进程
    if [ -e "server.pid" ]; then
        local pid
        pid=$(cat server.pid)
        print_color "$YELLOW" "Stopping $server (PID: $pid)..."
        
        if kill "$pid" 2>/dev/null; then
            # 等待进程结束
            local count=0
            while [ $count -lt 10 ] && kill -0 "$pid" 2>/dev/null; do
                sleep 1
                count=$((count + 1))
            done
            
            # 强制杀死如果还在运行
            if kill -0 "$pid" 2>/dev/null; then
                print_color "$YELLOW" "Force killing $server..."
                kill -9 "$pid"
            fi
        else
            print_color "$YELLOW" "Process $pid not running, cleaning up..."
        fi
        rm -f server.pid
    fi
    
    # 启动服务器
    print_color "$GREEN" "Starting $server..."
    if ../daeml "./$server"; then
        print_color "$GREEN" "$server started successfully"
    else
        print_color "$RED" "Failed to start $server"
    fi
    
    cd ..
}

# 停止服务
stop_server() {
    local server=$1
    local server_dir="./$server"
    
    if [ ! -d "$server_dir" ]; then
        print_color "$RED" "Error: Server directory $server_dir not found"
        return 1
    fi
    
    cd "$server_dir" || return 1
    
    if [ -e "server.pid" ]; then
        local pid
        pid=$(cat server.pid)
        print_color "$YELLOW" "Stopping $server (PID: $pid)..."
        
        if kill "$pid" 2>/dev/null; then
            local count=0
            while [ $count -lt 5 ] && kill -0 "$pid" 2>/dev/null; do
                sleep 1
                count=$((count + 1))
            done
            
            if kill -0 "$pid" 2>/dev/null; then
                print_color "$YELLOW" "Force killing $server..."
                kill -9 "$pid"
            fi
        fi
        
        rm -f server.pid
        print_color "$GREEN" "$server stopped"
    else
        print_color "$YELLOW" "$server is not running (no server.pid found)"
    fi
    
    cd ..
}

# 服务运行状态
status_server() {
    local server=$1
    local server_dir="./$server"
    
    if [ ! -d "$server_dir" ]; then
        print_color "$RED" "Error: Server directory $server_dir not found"
        return 1
    fi
    
    cd "$server_dir" || return 1
    
    if [ -e "server.pid" ]; then
        local pid
        pid=$(cat server.pid)
        if kill -0 "$pid" 2>/dev/null; then
            print_color "$GREEN" "$server is running (PID: $pid)"
        else
            print_color "$YELLOW" "$server pid file exists but process not running"
            rm -f server.pid
        fi
    else
        print_color "$RED" "$server is not running"
    fi
    
    cd ..
}

#########################################################################
## 文件同步函数
sync_libraries() {
    print_color "$BLUE" "Syncing libraries..."
    
    for server in "${ALL_SERVERS[@]}"; do
        if [ -d "./$server" ]; then
            cp -a ./lib/libslog.so "./$server/"
            cp -a ./lib/libprotobuf-lite* "./$server/"
            print_color "$GREEN" "Synced libraries to $server"
        fi
    done
}

sync_configurations() {
    print_color "$BLUE" "Syncing configurations..."
    
    # 同步日志配置
    for server in "${ALL_SERVERS[@]}"; do
        if [ -d "./$server" ]; then
            cp -a ./conf/log4cxx.properties "./$server/"
            print_color "$GREEN" "Synced log config to $server"
        fi
    done
    
    # 同步服务器配置文件
    local config_mappings=(
        "$LOGIN_SERVER_CONF:$LOGIN_SERVER"
        "$MSG_SERVER_CONF:$MSG_SERVER"
        "$ROUTE_SERVER_CONF:$ROUTE_SERVER"
        "$FILE_SERVER_CONF:$FILE_SERVER"
        "$MSFS_SERVER_CONF:$MSFS_SERVER"
        "$HTTP_MSG_SERVER_CONF:$HTTP_MSG_SERVER"
        "$PUSH_SERVER_CONF:$PUSH_SERVER"
        "$DB_PROXY_SERVER_CONF:$DB_PROXY_SERVER"
    )
    
    for mapping in "${config_mappings[@]}"; do
        local config="${mapping%:*}"
        local server="${mapping#*:}"
        if [ -f "./conf/$config" ] && [ -d "./$server" ]; then
            cp -f "./conf/$config" "./$server/"
            print_color "$GREEN" "Synced $config to $server"
        fi
    done
}

remove_libraries() {
    print_color "$YELLOW" "Removing libraries from server directories..."
    
    for server in "${ALL_SERVERS[@]}"; do
        if [ -d "./$server" ]; then
            rm -rf "./$server"/lib*
            print_color "$GREEN" "Removed libraries from $server"
        fi
    done
}

print_help() {
    echo "Usage: "
    echo "  $0 check --- check environment"
    echo "  $0 install --- check & run scripts to install"
}

## 主逻辑
setup_permissions() {
    chmod +x ./daeml ./restart.sh ./monitor.sh ./ttopen.sh 2>/dev/null || true
    print_color "$GREEN" "Set executable permissions"
}

print_help() {
    echo "TeamTalk Server Management Script"
    echo ""
    echo "Usage: $0 {start|stop|restart|status|install|check|clean}"
    echo ""
    echo "Commands:"
    echo "  start     - Start all servers"
    echo "  stop      - Stop all servers" 
    echo "  restart   - Restart all servers"
    echo "  status    - Show status of all servers"
    echo "  install   - Full installation (sync + start)"
    echo "  check     - Check environment"
    echo "  clean     - Remove libraries from server directories"
    echo "  sync      - Sync libraries and configurations only"
    echo ""
    echo "Single server operations:"
    echo "  $0 {start|stop|restart|status} <server_name>"
    echo ""
    echo "Available servers: ${ALL_SERVERS[*]}"
}

# 主程序
case "${1:-}" in
    start)
        print_header "Starting"
        check_user
        if [ -n "$2" ]; then
            restart_server "$2"
        else
            manage_all_servers "restart"
        fi
        ;;
    stop)
        print_header "Stopping"
        check_user
        if [ -n "$2" ]; then
            stop_server "$2"
        else
            manage_all_servers "stop"
        fi
        ;;
    restart)
        print_header "Restarting"
        check_user
        if [ -n "$2" ]; then
            restart_server "$2"
        else
            manage_all_servers "restart"
        fi
        ;;
    status)
        print_header "Status"
        if [ -n "$2" ]; then
            status_server "$2"
        else
            manage_all_servers "status"
        fi
        ;;
    install)
        print_header "Installing"
        check_user
        check_environment
        setup_permissions
        sync_libraries
        sync_configurations
        manage_all_servers "restart"
        ;;
    check)
        print_header "Environment Check"
        check_environment
        ;;
    clean)
        print_header "Cleaning"
        check_user
        remove_libraries
        ;;
    sync)
        print_header "Syncing"
        check_user
        sync_libraries
        sync_configurations
        ;;
    *)
        print_help
        exit 1
        ;;
esac

# function restart() {
#     cd $1
#     if [ ! -e *.conf  ]
#     then
#         echo "no config file"
#         return
#     fi

#     if [ -e server.pid  ]; then
#         pid=`cat server.pid`
#         echo "kill pid=$pid"
#         kill $pid
#         while true
#         do
#             oldpid=`ps -ef|grep $1|grep $pid`;
#             if [ $oldpid" " == $pid" " ]; then
#                 echo $oldpid" "$pid
#                 sleep 1
#             else
#                 break
#             fi
#         done
#         ../daeml ./$1
#     else 
#         ../daeml ./$1
#     fi
# }

# case $1 in
#     login_server)
#         restart $1
#         ;;
#     msg_server)
#         restart $1
#         ;;
#     route_server)
#         restart $1
#         ;;
#     http_msg_server)
#         restart $1
#         ;;
#     file_server)
#         restart $1
#         ;;
#     push_server)
#         restart $1
#         ;;
#     db_proxy_server)
#         restart $1
#         ;;
#     msfs)
#         if [ ! -d msfs/tmp ]
#         then
#             mkdir msfs/tmp
#         fi 
#         restart $1
#         ;;  
#     *)
#         echo "Usage: "
#         echo "  ./restart.sh (login_server|msg_server|route_server|http_msg_server|file_server|push_server|msfs)"
#         ;;
# esac

