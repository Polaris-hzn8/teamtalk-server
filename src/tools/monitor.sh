#!/bin/sh

#############################################################
# Server Process Monitor Script
# Automatically restart servers if they are not running
# Usage: ./monitor.sh [server_type] [port]
############################################################# 

# 服务器进程监控脚本 用于监控和自动重启服务 主要功能如下：
# 1.监控服务状态：定期检查指定服务是否在运行
# 2.自动重启：如果发现服务停止，自动重新启动
# 3.日志记录：记录服务重启的时间和原因
# 4.告警通知：通过短信发送服务异常通知

# Configuration
MONITOR_INTERVAL=15
RESTART_LOG="restart.log"
PID_FILE="monitor.pid"
OLD_LOG_DIR="oldlog"

# SMS notification function (to be implemented)
send_sms() {
    echo "SMS Alert: Server $1 is not running" >&2
    # Add actual SMS sending logic here
    # Example: curl to SMS API
}

# Log function
log_message() {
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $1" >> "$RESTART_LOG"
}

# Check required directories
setup_environment() {
    if [ ! -d "$OLD_LOG_DIR" ]; then
        mkdir -p "$OLD_LOG_DIR"
    fi
}

# Monitor function for standard servers
monitor_server() {
    local server_name="$1"
    
    if [ ! -e *.conf ]; then 
        echo "Error: No config file found"
        return 1
    fi
    
    echo $$ > monitor.pid
    setup_environment

    if [ -e log*_1.txt ]; then
        while true; do 
            local log_file=$(ls log*_1.txt 2>/dev/null | head -1)
            if [ -z "$log_file" ]; then
                sleep "$MONITOR_INTERVAL"
                continue
            fi
            
            local pid=$(echo "$log_file" | awk -F_ '{print $2}')
            local process_count=$(ps aux | grep "$server_name" | grep "$pid" | grep -v grep | wc -l)
            
            if [ "$process_count" -eq 0 ]; then
                # Send alert
                send_sms "$server_name"
                
                # Log restart event
                log_message "Server $server_name stopped, pid=$pid, restarting..."
                
                # Move old logs and restart server
                mv log*.txt "$OLD_LOG_DIR/" 2>/dev/null
                ../daeml "./$server_name"
                
                log_message "Server $server_name restarted successfully"
            fi

            sleep "$MONITOR_INTERVAL"
        done
    else
        echo "Error: No log files found for monitoring"
        return 1
    fi
}

# Monitor function for business server
monitor_business() {
    local port="$1"
    
    if [ -z "$port" ]; then 
        echo "Usage: ./monitor.sh business <port>"
        exit 1
    fi

    setup_environment

    while true; do 
        if [ ! -f "$PID_FILE" ]; then
            log_message "PID file not found for business server, restarting..."
            ./run.sh "$port"
            sleep "$MONITOR_INTERVAL"
            continue
        fi
        
        local pid=$(cat "$PID_FILE" 2>/dev/null)
        local process_count=0
        
        if [ -n "$pid" ]; then
            process_count=$(ps aux | grep "mogutalk-business" | grep "$pid" | grep -v grep | wc -l)
        fi
        
        if [ "$process_count" -eq 0 ]; then 
            # Send alert
            send_sms "mogutalk-business"
            
            # Log restart event
            log_message "Business server stopped, restarting on port $port..."
            
            # Restart server
            ./run.sh "$port"
            
            log_message "Business server restarted on port $port"
        fi
        
        sleep "$MONITOR_INTERVAL"
    done
}

# Test function
test_monitor() {
    echo "Testing monitor script..."
    send_sms "TEST"
    echo "Test completed. Check restart.log for details."
}

# Main script
case $1 in
    login_server|msg_server|route_server|http_msg_server)
        monitor_server "$1"
        ;;
    business)
        monitor_business "$2"
        ;;
    test)
        test_monitor
        ;;
    *)
        echo "Usage: "
        echo "  ./monitor.sh (login_server|msg_server|route_server|http_msg_server)"
        echo "  ./monitor.sh business <port>"
        echo "  ./monitor.sh test"
        echo ""
        echo "Description: Monitor and automatically restart server processes"
        ;;
esac

