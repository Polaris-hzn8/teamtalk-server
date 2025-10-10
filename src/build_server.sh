#!/bin/bash

set -e

# ================================================
# TeamTalk Server Build & Packaging Script
# 支持：
#   1. 全量编译打包：   ./build_server.sh version <version>
#   2. 仅打包已编译结果：./build_server.sh pack <version>
#   3. 清理：           ./build_server.sh clean
#   4. 单独编译某个 server
# ================================================

# 配置变量
LIB_DIR="lib"
PACK_FOLDER_NAME="im_server_pack"

# 全局变量（打包目录和包名）
PACK_DIR=""
TARGET_NAME=""

# 服务器列表
SERVERS=(
    "base"
    "login_server"
    "msg_server"
    "http_msg_server"
    "file_server"
    "route_server"
    "db_proxy_server"
    "msfs"
    "push_server")

# ================================================
# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'
color_echo() {
    echo -e "${1}${2}${NC}"
}

# ================================================
# 环境设置
setup_environment() {
    local CURPWD=$PWD
    # 基础路径
    export CPLUS_INCLUDE_PATH=$CURPWD/base:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$CURPWD/base/bin:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$CURPWD/base/bin:$LIBRARY_PATH
    
    # slog
    export CPLUS_INCLUDE_PATH=$CURPWD/third/slog/include:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$CURPWD/third/slog/lib:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$CURPWD/third/slog/lib:$LIBRARY_PATH
    
    # protobuf
    export CPLUS_INCLUDE_PATH=$CURPWD/third/protobuf/include:$CPLUS_INCLUDE_PATH
    export LD_LIBRARY_PATH=$CURPWD/third/protobuf/lib:$LD_LIBRARY_PATH
    export LIBRARY_PATH=$CURPWD/third/protobuf/lib:$LIBRARY_PATH
}

# ================================================
# 文件检查
check_required_files() {
    local missing_files=()
    
    # 检查配置文件
    [ ! -f "third/slog/log4cxx.properties" ] && missing_files+=("third/slog/log4cxx.properties")
    
    # 检查服务器配置文件（base不需要配置文件）
    for server in "${SERVERS[@]}"; do
        if [ "$server" != "base" ]; then
            local conf_file="${server}/${server}.conf"
            [ ! -f "$conf_file" ] && missing_files+=("$conf_file")
        fi
    done
    
    if [ ${#missing_files[@]} -ne 0 ]; then
        echo "Error: Missing required files:"
        printf '  %s\n' "${missing_files[@]}"
        return 1
    fi
    
    return 0
}

# ================================================
# 创建版本文件
create_version_file() {
    local version=$1
    cat > base/version.h << EOF
#ifndef __VERSION_H__
#define __VERSION_H__
#define VERSION "$version"
#endif
EOF
    echo ">>> Created version.h with version: $version"
}

# ================================================
# 构建单个服务器
build_single_server() {
    local server=$1
    local CURPWD=$PWD
    
    echo ">>> Building $server..."
    
    if [ ! -d "$server" ]; then
        echo "Error: Server directory '$server' not found"
        return 1
    fi
    
    cd "$server"
    
    # 清理并创建构建目录
    rm -rf build bin
    mkdir -p build
    cd build
    
    # 编译
    if cmake ../ && make; then
        echo ">>> Build $server: SUCCESS"
    else
        echo ">>> Build $server: FAILED"
        return 1
    fi
    
    cd "$CURPWD"
    return 0
}

# ================================================
# 构建所有服务器
build_all_servers() {
    echo ">>> Building all servers..."
    
    for server in "${SERVERS[@]}"; do
        if ! build_single_server "$server"; then
            return 1
        fi
    done
    
    echo ">>> All servers built successfully"
    return 0
}

# ================================================
# 准备打包目录
prepare_pack_directory() {
    local version=$1

    PACK_DIR="../${PACK_FOLDER_NAME}"
    TARGET_NAME="im-server-${version}.tar.gz"
    
    echo ">>> Preparing pack directory..."
    
    # 清理旧文件
    rm -rf "$PACK_DIR"
    rm -f "$TARGET_NAME"
    
    # 创建目录结构（base不需要在打包目录中创建子目录）
    mkdir -p "$PACK_DIR/$LIB_DIR"

    # 为其他服务器创建目录
    for server in "${SERVERS[@]}"; do
        if [ "$server" != "base" ]; then
            mkdir -p "$PACK_DIR/$server"
        fi
    done

    echo "$PACK_DIR"
    echo "$TARGET_NAME"
}

# ================================================
# 复制文件到打包目录
copy_files_to_pack() {
    echo ">>> Copying files to pack directory..."
    
    LOG4CXX_CONFIG="third/slog/log4cxx.properties"
    for server in "${SERVERS[@]}"; do
        if [ "$server" != "base" ] && [ -f "$server/bin/$server" ]; then
            # 复制可执行文件
            cp "$server/bin/$server" "$PACK_DIR/$server/"
            echo "  Copied $server/bin/$server"
            # 复制服务器配置文件
            cp "$server/${server}.conf" "$PACK_DIR/$server/"
            echo "  Copied $server/${server}.conf"
            # 复制日志配置文件
            cp "$LOG4CXX_CONFIG" "$PACK_DIR/$server/"
            echo "  Copied $server/log4cxx.properties"
        fi
    done
    
    #######################################################################
    # 复制库文件
    # slog
    SLOG_LIB="third/slog/lib/libslog.so"
    if [ -f "$SLOG_LIB" ]; then
        cp "$SLOG_LIB" "$PACK_DIR/$LIB_DIR/"
        echo "  Copied $SLOG_LIB"
    fi

    # log4cxx
    LOG4CXX_DIR="third/log4cxx/lib"
    LOG4CXX_LIBS=("liblog4cxx.so.10" "liblog4cxx.so.10.0.0")
    for f in "${LOG4CXX_LIBS[@]}"; do
        if [ -f "$LOG4CXX_DIR/$f" ]; then
            cp "$LOG4CXX_DIR/$f" "$PACK_DIR/$LIB_DIR/"
            echo "  Copied $LOG4CXX_DIR/$f"
        else
            echo "Warning: $f not found in $LOG4CXX_DIR!"
        fi
    done
    
    # protobuff
    PROTOBUF_DIR="third/protobuf/lib"
    PROTOBUF_LIBS=("libprotobuf-lite.so.9" "libprotobuf-lite.so.9.0.1"
                    "libprotobuf.so.9" "libprotobuf.so.9.0.1")
    for f in "${PROTOBUF_LIBS[@]}"; do
        if [ -f "$PROTOBUF_DIR/$f" ]; then
            cp "$PROTOBUF_DIR/$f" "$PACK_DIR/$LIB_DIR/"
            echo "  Copied $PROTOBUF_DIR/$f"
        else
            echo "Warning: $f not found in $PROTOBUF_DIR!"
        fi
    done

    # 复制脚本文件
    for script in scripts/server_manager.sh scripts/server_monitor.sh scripts/init.sh scripts/init.sql; do
        if [ -f "$script" ]; then
            cp "$script" "$PACK_DIR/"
            echo "  Copied $script"
        fi
    done

    # 构建并复制daeml
    if [ -d "daeml" ]; then
        color_echo "$BLUE" ">>> Rebuilding daeml..."
        echo ">>> Building daeml..."
        if make -C daeml clean && make -C daeml; then
            cp "daeml/daeml" "$PACK_DIR/"
            color_echo "$GREEN" "  daeml built successfully"
            color_echo "$GREEN" "  Copied daeml/daeml"
        else
            color_echo "$YELLOW" "Warning: Failed to build daeml"
        fi
    fi
}

# ================================================
# 创建压缩包
create_package() {
    echo ">>> Creating package: $TARGET_NAME"
    
    if tar zcvf "$TARGET_NAME" -C "$(dirname "$PACK_DIR")" "$(basename "$PACK_DIR")"; then
        color_echo "$GREEN" ">>> Package created successfully: $TARGET_NAME"
        return 0
    else
        color_echo "$YELLOW" ">>> Failed to create package"
        return 1
    fi
}

# ================================================
# 全量构建 + 打包
build_pack() {
    local version=$1
    local CURPWD=$PWD
    
    echo ">>> Starting build process for version: $version"
    echo ">>> Current directory: $CURPWD"
    
    # 设置环境
    setup_environment
    
    # 检查必要文件
    if ! check_required_files; then
        echo ">>> Build failed: Required files missing"
        return 1
    fi
    
    # 创建版本文件
    create_version_file "$version"
    
    # 构建所有服务器
    if ! build_all_servers; then
        echo ">>> Build failed: Server compilation error"
        return 1
    fi
    
    # 准备打包
    prepare_pack_directory "$version"
    
    # 复制文件
    copy_files_to_pack

    # 调试使用
    chmod +x ../im_server_pack/server_manager.sh

    # 创建压缩包
    if create_package; then
        echo ">>> Build completed successfully!"
        return 0
    else
        return 1
    fi
}

# ================================================
# 仅打包现有结果（不编译 server）
pack_existing() {
    local version=$1
    local CURPWD=$PWD

    color_echo "$BLUE" ">>> Packaging existing build for version: $version"
    setup_environment

    if ! check_required_files; then
        color_echo "$RED" ">>> Packaging failed: Required files missing"
        return 1
    fi

    # 创建版本文件
    create_version_file "$version"

    # 准备打包
    prepare_pack_directory "$version"

    # 复制文件
    copy_files_to_pack

    # 调试使用
    chmod +x ../im_server_pack/server_manager.sh

    # 创建压缩包
    if create_package; then
        color_echo "$GREEN" ">>> quick_pack succeed!"
        return 0
    else
        return 1
    fi
}

# 清理函数
pack_clean() {
    local CURPWD=$PWD
    echo ">>> Cleaning pack files..."
    cd ..
    rm -f im-server-*
    rm -rf "$PACK_FOLDER_NAME"
    echo ">>> Pack cleanup completed"
    cd $CURPWD
}

cmake_clean() {
    echo ">>> Cleaning build directories..."
    make -C daeml clean
    for server in "${SERVERS[@]}"; do
        if [ -d "$server" ]; then
            cd "$server"
            rm -rf build bin
            cd ..
            echo "  Cleaned $server"
        fi
    done
    echo ">>> Build cleanup completed"
}

# 显示帮助信息
print_help() {
    cat << EOF
Usage:
  $0 clean                --- clean all build files
  $0 version <version>    --- build complete package with version (compile all servers)
  $0 pack <version>       --- package existing binaries (rebuild daeml only)
  $0 <server_name>        --- build single server

Available servers:
  ${SERVERS[*]}

Examples:
  $0 version 1.0.0        # Build complete package version 1.0.0
  $0 pack 1.0.0           # Only package existing binaries
  $0 clean                # Clean all build files
  $0 login_server         # Build only login_server
  $0 base                 # Build only base library
EOF
}

# 主程序
main() {
    case $1 in
        clean)
            echo ">>> Cleaning all build files..."
            cmake_clean
            pack_clean
            ;;
        version)
            if [ $# -ne 2 ]; then
                echo "Error: Version number required"
                print_help
                exit 1
            fi
            echo ">>> Building version: $2"
            cmake_clean
            pack_clean
            build_pack "$2"
            ;;
        pack)
            [ $# -ne 2 ] && { color_echo "$RED" "Error: Version number required"; print_help; exit 1; }
            pack_clean
            pack_existing "$2"
            ;;
        *)
            if [[ " ${SERVERS[@]} " =~ " $1 " ]]; then
                build_single_server "$1"
            else
                print_help
                exit 1
            fi
            ;;
    esac
}

# 运行主程序
main "$@"
