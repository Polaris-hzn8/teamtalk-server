#!/bin/bash
# Teamtalk Server Management Script
# 服务器进程组管理脚本

#########################################################################
## 服务器配置变量
readonly FILE_SERVER=file_server
readonly LOGIN_SERVER=login_server
readonly MSG_SERVER=msg_server
readonly ROUTE_SERVER=route_server
readonly HTTP_MSG_SERVER=http_msg_server
readonly PUSH_SERVER=push_server
readonly DB_PROXY_SERVER=db_proxy_server
readonly MSFS_SERVER=msfs

readonly FILE_SERVER_CONF=file_server.conf
readonly LOGIN_SERVER_CONF=login_server.conf
readonly MSG_SERVER_CONF=msg_server.conf
readonly ROUTE_SERVER_CONF=route_server.conf
readonly HTTP_MSG_SERVER_CONF=http_msg_server.conf
readonly PUSH_SERVER_CONF=push_server.conf
readonly DB_PROXY_SERVER_CONF=db_proxy_server.conf
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

#########################################################################
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

print_header(){
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
    if [ ! -d "./lib" ]; then
        print_color "$RED" "Error: lib or conf directory not found"
        exit 1
    fi
}

#########################################################################
## 设置权限
setup_permissions() {
    chmod +x ./server_manager.sh ./server_monitor.sh ./init.sh ./daeml 2>/dev/null || true
    # 所有 server 可执行文件
    for srv in "${ALL_SERVERS[@]}"; do
        if [ -f "./$srv/$srv" ]; then
            chmod +x "./$srv/$srv"
        fi
    done
    print_color "$GREEN" "Set executable permissions"
}

#########################################################################
## 服务操作函数
restart_server() {
    local server=$1
    local server_dir="./$server"
    local deaml="$(pwd)/daeml"

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
    if "$deaml" "./$server"; then
        print_color "$GREEN" "$server started successfully"
    else
        print_color "$RED" "Failed to start $server"
    fi

    cd ..
}

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

manage_all_servers() {
    local action=$1
    local failed=()
    for srv in "${ALL_SERVERS[@]}"; do
        print_color "$BLUE" "=== $action $srv ==="
        if ! ${action}_server "$srv"; then
            failed+=("$srv")
        fi
        echo
    done
    if [ ${#failed[@]} -gt 0 ]; then
        print_color "$YELLOW" "Warning: The following servers had issues: ${failed[*]}"
    fi
}

#########################################################################
## 主逻辑
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
    echo "  clean     - Remove server config from server directories"
    echo "  sync      - Sync server configurations only"
    echo ""
    echo "Single server operations:"
    echo "  $0 {start|stop|restart|status} <server_name>"
    echo ""
    echo "Available servers: ${ALL_SERVERS[*]}"
}

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
        manage_all_servers "restart"
        ;;
    check)
        print_header "Environment Check"
        check_environment
        ;;
    clean)
        print_header "Cleaning"
        check_user
        ;;
    *)
        print_help
        exit 1
        ;;
esac

