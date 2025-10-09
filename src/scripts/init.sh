#!/bin/bash
# MySQL Database Initialization Script for TeamTalk

set -e  # 遇到错误立即退出

SQL_FILE="init.sql"
MYSQL_HOST="localhost"
MYSQL_USER="root"
MYSQL_PASSWORD=20001201

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo "==========================================="
    echo -e "${BLUE}$1 - MySQL Table Initialization${NC}"
    echo "==========================================="
}

check_user() {
    if [ $(id -u) != "0" ]; then
        echo -e "${RED}Error: You must be root to run this script${NC}"
        exit 1
    fi
}

check_sql_file() {
    if [ ! -f "$SQL_FILE" ]; then
        echo -e "${RED}Error: SQL file '$SQL_FILE' not found${NC}"
        return 1
    fi
}

safe_password_input() {
    if [ -z "$MYSQL_PASSWORD" ]; then
        read -s -p "Enter MySQL root password: " MYSQL_PASSWORD
        echo
        if [ -z "$MYSQL_PASSWORD" ]; then
            echo -e "${RED}Error: MySQL password is required${NC}"
            exit 1
        fi
    fi
}

check_mysql_connection() {
    if ! mysql -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" -e "SELECT 1;" >/dev/null 2>&1; then
        echo -e "${RED}Error: Cannot connect to MySQL${NC}"
        return 1
    fi
}

create_database() {
    echo "Initializing database..."
    
    if mysql -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" -h "$MYSQL_HOST" < "$SQL_FILE"; then
        echo -e "${GREEN}Database initialized successfully${NC}"
    else
        echo -e "${RED}Error: Failed to initialize database${NC}"
        return 1
    fi
}

check_environment() {
    print_header "Environment Check"
    check_user
    safe_password_input
    check_mysql_connection || exit 1
    check_sql_file || exit 1
    echo -e "${GREEN}Environment check passed${NC}"
}

install_database() {
    print_header "Installing"
    check_user
    safe_password_input
    check_mysql_connection || exit 1
    check_sql_file || exit 1
    create_database || exit 1
    echo -e "${GREEN}MySQL initialization completed!${NC}"
}

print_help() {
    echo "Usage: "
    echo "  $0 check     --- check environment"
    echo "  $0 install   --- initialize database"
    echo "  $0 help      --- show this help"
    echo ""
    echo "Note: Set MYSQL_PASSWORD environment variable for automation"
}

case $1 in
    check)
        check_environment
        ;;
    install)
        install_database
        ;;
    help|--help|-h)
        print_help
        ;;
    *)
        print_help
        exit 1
        ;;
esac
