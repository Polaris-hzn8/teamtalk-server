#!/bin/bash
# Teamtalk Server Management Script
# 服务器进程组管理脚本

#########################################################################
## 脚本目录
BASE_DIR="$(cd "$(dirname "$0")" && pwd)"

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
    if [ ! -d "$BASE_DIR/lib" ]; then
        print_color "$RED" "Error: lib directory not found"
        exit 1
    fi
}

#########################################################################
## 设置权限
setup_permissions() {
    chmod +x "$BASE_DIR/server_manager.sh" "$BASE_DIR/server_monitor.sh" "$BASE_DIR/init.sh" "$BASE_DIR/daeml" 2>/dev/null || true
    # 所有 server 可执行文件
    for srv in "${ALL_SERVERS[@]}"; do
        if [ -f "$BASE_DIR/$srv/$srv" ]; then
            chmod +x "$BASE_DIR/$srv/$srv"
        fi
    done
    print_color "$GREEN" "Set executable permissions"
}

#########################################################################
## 清理日志和core文件
clean_logs() {
    print_header "Cleaning logs and core dumps"

    local cleaned=0
    for srv in "${ALL_SERVERS[@]}"; do
        local srv_dir="$BASE_DIR/$srv"

        if [ -d "$srv_dir" ]; then
            # 删除日志文件
            find "$srv_dir/log" -type f -name "*.log" -o -name "*.txt" 2>/dev/null | while read -r logf; do
                rm -f "$logf" && print_color "$GREEN" "Removed log file: $logf"
                cleaned=1
            done

            # 删除 core 文件
            find "$srv_dir" -maxdepth 1 -type f -name "core.*" 2>/dev/null | while read -r coref; do
                rm -f "$coref" && print_color "$YELLOW" "Removed core dump: $coref"
                cleaned=1
            done
        fi
    done

    if [ "$cleaned" -eq 0 ]; then
        print_color "$BLUE" "No log or core files found."
    else
        print_color "$GREEN" "All logs and core dumps cleaned successfully."
    fi
}

#########################################################################
## 服务操作函数
restart_server() {
    local server=$1
    local server_dir="$BASE_DIR/$server"
    local deaml="$BASE_DIR/daeml"

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
    if "$deaml" "$BASE_DIR/$server/$server"; then
        print_color "$GREEN" "$server started successfully"
    else
        print_color "$RED" "Failed to start $server"
    fi

    cd "$BASE_DIR"
}

stop_server() {
    local server=$1
    local server_dir="$BASE_DIR/$server"

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

    cd "$BASE_DIR"
}

force_kill_server() {
    local server=$1
    local killed=0

    # 查找匹配的进程
    local pids
    pids=$(ps -ef | grep "$server" | grep -v grep | grep "$BASE_DIR" | awk '{print $2}')

    if [ -n "$pids" ]; then
        print_color "$YELLOW" "Force killing $server (PIDs: $pids)"
        kill -9 $pids 2>/dev/null && print_color "$GREEN" "$server killed"
        killed=1
    else
        print_color "$BLUE" "$server is not running"
    fi

    # 清理遗留 pid 文件
    if [ -f "$BASE_DIR/$server/server.pid" ]; then
        rm -f "$BASE_DIR/$server/server.pid"
        print_color "$GREEN" "Removed stale pid file for $server"
    fi

    return $killed
}

status_server() {
    local server=$1
    local server_dir="$BASE_DIR/$server"

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

    cd "$BASE_DIR"
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
    echo "Usage: $0 {start|stop|restart|status|install|check|clean|clean_logs}"
    echo ""
    echo "Commands:"
    echo "  start     - Start all servers"
    echo "  stop      - Stop all servers"
    echo "  kill      - Force kill all server processes (or single server)"
    echo "  restart   - Restart all servers"
    echo "  status    - Show status of all servers"
    echo "  install   - Full installation (sync + start)"
    echo "  check     - Check environment"
    echo "  clean     - Remove server config from server directories"
    echo "  clean_logs  Clean all log/*.log and core.* files"
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
    kill)
        print_header "Force Killing"
        check_user
        if [ -n "$2" ]; then
            force_kill_server "$2"
        else
            manage_all_servers "force_kill"
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

        print_color "$YELLOW" "Killing all existing servers before installation..."
        manage_all_servers "force_kill"

        print_color "$GREEN" "All old server processes have been killed. Starting fresh installation..."
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
    clean_logs)
        clean_logs
        ;;
    *)
        print_help
        exit 1
        ;;
esac

