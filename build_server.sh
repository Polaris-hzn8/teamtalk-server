#!/bin/bash

set -e

# ================================================
# TeamTalk Server Build & Packaging Script
# ================================================

# ========== 颜色配置 ==========
COLOR_RED='\033[0;31m'
COLOR_GREEN='\033[0;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_BLUE='\033[0;34m'
COLOR_NC='\033[0m'

# ========== 根目录 ==========
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="${ROOT_DIR}/src"

# ========== 打包配置 ==========
LIB_DIR="lib"
PACK_FOLDER_NAME="im_server_pack"
PACKAGE_PREFIX="im-server"

# ========== 服务器列表 ==========
SERVERS=(
    "base"
    "login_server"
    "msg_server"
    "http_msg_server"
    "file_server"
    "route_server"
    "db_proxy_server"
    "msfs"
    "push_server"
)

# ========== 第三方库配置 ==========
SLOG_DIR="third/slog"
SLOG_LIB="libslog.so"
SLOG_INCLUDE_DIR="${SLOG_DIR}/include"

LOG4CXX_DIR="third/log4cxx"
LOG4CXX_LIBS=("liblog4cxx.so.10" "liblog4cxx.so.10.0.0")
LOG4CXX_CONFIG="${SRC_DIR}/${SLOG_DIR}/log4cxx.properties"

PROTOBUF_DIR="third/protobuf"
PROTOBUF_INC_DIR="third/protobuf/include"
PROTOBUF_LIBS=(
    "libprotobuf-lite.so.9"
    "libprotobuf-lite.so.9.0.1"
    "libprotobuf.so.9"
    "libprotobuf.so.9.0.1"
)

# ========== 打包脚本配置 ==========
PACK_SCRIPTS=(
    "scripts/server_manager.sh"
    "scripts/server_monitor.sh"
    "scripts/init.sh"
    "scripts/init.sql"
)

# ========== 其他工具配置 ==========
DAEML_DIR="daeml"
DAEML_BIN="daeml"

# ========== 全局变量 ==========
PACK_DIR=""
TARGET_NAME=""

# ================================================
# 日志输出函数
# ================================================
log_info() {
    echo -e "${COLOR_BLUE}>>> $*${COLOR_NC}"
}

log_success() {
    echo -e "${COLOR_GREEN}>>> $*${COLOR_NC}"
}

log_error() {
    echo -e "${COLOR_RED}>>> $*${COLOR_NC}" >&2
}

log_warn() {
    echo -e "${COLOR_YELLOW}>>> $*${COLOR_NC}"
}

# ================================================
# 环境设置
# ================================================
setup_environment() {
    export LD_LIBRARY_PATH="${SRC_DIR}/base/bin:${SRC_DIR}/${SLOG_DIR}/lib:${SRC_DIR}/${PROTOBUF_DIR}/lib:${LD_LIBRARY_PATH}"
}

# ================================================
# 文件检查
# ================================================
check_required_files() {
    local missing_files=()

    # 检查日志配置文件
    [ ! -f "${LOG4CXX_CONFIG}" ] && missing_files+=("${LOG4CXX_CONFIG}")

    # 检查服务器配置文件
    for server in "${SERVERS[@]}"; do
        if [ "$server" != "base" ]; then
            local conf_file="${SRC_DIR}/${server}/${server}.conf"
            [ ! -f "$conf_file" ] && missing_files+=("$conf_file")
        fi
    done

    if [ ${#missing_files[@]} -ne 0 ]; then
        log_error "Missing required files:"
        printf '  %s\n' "${missing_files[@]}"
        return 1
    fi

    return 0
}

# ================================================
# 创建版本文件
# ================================================
create_version_file() {
    local version=$1

    cat > "${SRC_DIR}/base/version.h" << EOF
#ifndef __VERSION_H__
#define __VERSION_H__
#define VERSION "$version"
#endif
EOF
    log_info "Created version.h with version: $version"
}

# ================================================
# 构建单个服务
# ================================================
build_single_server() {
    local server=$1
    local server_dir="${SRC_DIR}/${server}"
    local build_dir="${server_dir}/build"

    log_info "Building $server..."

    if [ ! -d "$server_dir" ]; then
        log_error "Server directory '$server_dir' not found"
        return 1
    fi

    rm -rf "$build_dir"
    mkdir -p "$build_dir"

    # 切换到 build 目录执行构建
    if (cd "$build_dir" && cmake "$server_dir" && make); then
        log_success "Build $server: SUCCESS"
    else
        log_error "Build $server: FAILED"
        return 1
    fi
}

# ================================================
# 构建所有服务
# ================================================
build_all_servers() {
    log_info "Building all servers..."

    for server in "${SERVERS[@]}"; do
        if ! build_single_server "$server"; then
            return 1
        fi
    done

    log_success "All servers built successfully"
    return 0
}

# ================================================
# 准备打包目录并复制文件
# ================================================
prepare_pack_directory() {
    local version=$1

    PACK_DIR="${ROOT_DIR}/${PACK_FOLDER_NAME}"
    TARGET_NAME="${ROOT_DIR}/${PACKAGE_PREFIX}-${version}.tar.gz"

    log_info "Preparing pack directory..."

    rm -rf "$PACK_DIR"
    rm -f "$TARGET_NAME"

    mkdir -p "$PACK_DIR/${LIB_DIR}"

    # 复制服务器可执行文件、配置及日志配置
    for server in "${SERVERS[@]}"; do
        if [ "$server" != "base" ]; then
            mkdir -p "$PACK_DIR/$server"

            if [ -f "${SRC_DIR}/${server}/bin/${server}" ]; then
                cp "${SRC_DIR}/${server}/bin/${server}" "${PACK_DIR}/${server}/"
                log_info "  Copied ${server}/bin/${server}"
            fi

            if [ -f "${SRC_DIR}/${server}/${server}.conf" ]; then
                cp "${SRC_DIR}/${server}/${server}.conf" "${PACK_DIR}/${server}/"
                log_info "  Copied ${server}/${server}.conf"
            fi

            cp "${LOG4CXX_CONFIG}" "${PACK_DIR}/${server}/"
            log_info "  Copied ${server}/log4cxx.properties"
        fi
    done

    # 复制 slog 库
    local slog_lib="${SRC_DIR}/${SLOG_DIR}/lib/${SLOG_LIB}"
    if [ -f "$slog_lib" ]; then
        cp "$slog_lib" "${PACK_DIR}/${LIB_DIR}/"
        log_info "  Copied $slog_lib"
    fi

    # 复制 log4cxx 库
    for lib in "${LOG4CXX_LIBS[@]}"; do
        local lib_path="${SRC_DIR}/${LOG4CXX_DIR}/lib/${lib}"
        if [ -f "$lib_path" ]; then
            cp "$lib_path" "${PACK_DIR}/${LIB_DIR}/"
            log_info "  Copied ${LOG4CXX_DIR}/lib/${lib}"
        else
            log_warn "${lib} not found in ${LOG4CXX_DIR}/lib!"
        fi
    done

    # 复制 protobuf 库
    for lib in "${PROTOBUF_LIBS[@]}"; do
        local lib_path="${SRC_DIR}/${PROTOBUF_DIR}/lib/${lib}"
        if [ -f "$lib_path" ]; then
            cp "$lib_path" "${PACK_DIR}/${LIB_DIR}/"
            log_info "  Copied ${PROTOBUF_DIR}/lib/${lib}"
        else
            log_warn "${lib} not found in ${PROTOBUF_DIR}/lib!"
        fi
    done

    # 复制脚本文件
    for script in "${PACK_SCRIPTS[@]}"; do
        if [ -f "${SRC_DIR}/${script}" ]; then
            cp "${SRC_DIR}/${script}" "${PACK_DIR}/"
            log_info "  Copied $script"
        fi
    done

    # 构建并复制 daeml
    if [ -d "${SRC_DIR}/${DAEML_DIR}" ]; then
        log_info "Rebuilding daemon..."
        if make -C "${SRC_DIR}/${DAEML_DIR}" clean && make -C "${SRC_DIR}/${DAEML_DIR}"; then
            cp "${SRC_DIR}/${DAEML_DIR}/${DAEML_BIN}" "${PACK_DIR}/"
            log_success "  daeml built and copied"
        else
            log_warn "Failed to build daeml"
        fi
    fi

    echo "$PACK_DIR"
    echo "$TARGET_NAME"
}

# ================================================
# 创建压缩包
# ================================================
create_package() {
    log_info "Creating package: $TARGET_NAME"

    if tar zcvf "$TARGET_NAME" -C "$(dirname "$PACK_DIR")" "$(basename "$PACK_DIR")"; then
        log_success "Package created successfully: $TARGET_NAME"
        return 0
    else
        log_warn "Failed to create package"
        return 1
    fi
}

# ================================================
# 清理打包文件
# ================================================
clean_pack() {
    log_info "Cleaning pack files..."

    rm -f "${ROOT_DIR}/${PACKAGE_PREFIX}-"*.tar.gz
    rm -rf "${ROOT_DIR}/${PACK_FOLDER_NAME}"

    log_success "Pack cleanup completed"
}

# ================================================
# 清理编译产物
# ================================================
clean_build() {
    log_info "Cleaning build directories..."

    if [ -d "${SRC_DIR}/${DAEML_DIR}" ]; then
        make -C "${SRC_DIR}/${DAEML_DIR}" clean
    fi

    for server in "${SERVERS[@]}"; do
        if [ -d "${SRC_DIR}/${server}" ]; then
            rm -rf "${SRC_DIR}/${server}/build" "${SRC_DIR}/${server}/bin"
            log_info "  Cleaned $server"
        fi
    done

    log_success "Build cleanup completed"
}

# ================================================
# 显示帮助信息
# ================================================
print_help() {
    cat << EOF
TeamTalk Server Build Script

Usage:
  $0 build <server>       --- build single server
  $0 build_version <version>  --- build all servers and create package
  $0 clean                --- clean all build files

Available servers:
  ${SERVERS[*]}

Examples:
  $0 build login_server   # Build only login_server
  $0 build_version 1.0.0  # Build all servers and package
  $0 clean                # Clean all build files

Configuration:
  Pack folder: $PACK_FOLDER_NAME
  Package prefix: $PACKAGE_PREFIX
  Lib directory: $LIB_DIR
EOF
}

# ===============================================================
# 模式1：全量构建 + 打包
# ===============================================================
build_and_pack() {
    local version=$1

    log_info "Building version: $version"
    log_info "Current directory: $ROOT_DIR"

    setup_environment

    if ! check_required_files; then
        log_error "Build failed: Required files missing"
        return 1
    fi

    create_version_file "$version"

    if ! build_all_servers; then
        log_error "Build failed: Server compilation error"
        return 1
    fi

    prepare_pack_directory "$version"

    chmod +x "${PACK_DIR}/server_manager.sh"

    if create_package; then
        log_success "Build completed successfully!"
        return 0
    else
        return 1
    fi
}

# ===============================================================
# 模式2：打包已有构建
# ===============================================================
pack_existing_build() {
    local version=$1

    log_info "Packing existing build for version: $version"
    log_info "Current directory: $ROOT_DIR"

    prepare_pack_directory "$version"

    chmod +x "${PACK_DIR}/server_manager.sh"

    if create_package; then
        log_success "Pack completed successfully!"
        return 0
    else
        return 1
    fi
}

# ================================================
# 主程序
# ================================================
main() {
    if [[ ! -d "$SRC_DIR" ]]; then
        log_error "Expected source tree at: $SRC_DIR"
        exit 1
    fi

    case $1 in
        build)
            if [ $# -ne 2 ]; then
                log_error "Usage: $0 build <server>"
                exit 1
            fi
            setup_environment
            build_single_server "$2"
            ;;
        build_version)
            if [ $# -ne 2 ]; then
                log_error "Usage: $0 build_version <version>"
                exit 1
            fi
            setup_environment
            clean_build
            clean_pack
            build_and_pack "$2"
            ;;
        clean)
            clean_build
            clean_pack
            ;;
        *)
            print_help
            exit 1
            ;;
    esac
}

main "$@"