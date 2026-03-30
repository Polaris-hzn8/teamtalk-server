#!/bin/bash
# 兼容旧用法：从 src 目录调用时转发到仓库根目录的 build_server.sh。
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/../build_server.sh" "$@"
