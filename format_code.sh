#!/usr/bin/env bash
# Format C/C++ sources under src/ with Google style (via .clang-format in repo root).
# Usage: ./format_code.sh   (from repository root)
# Requires: clang-format (set CLANG_FORMAT to a specific binary if needed).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CF="${CLANG_FORMAT:-clang-format}"

if ! command -v "$CF" >/dev/null 2>&1; then
  echo "error: '$CF' not found. Install LLVM clang-format or set CLANG_FORMAT." >&2
  exit 1
fi

# Third-party and generated trees: skip to avoid massive unrelated diffs.
# Add more -path '*/something/*' prunes if needed.
mapfile -d '' FILES < <(
  find "$ROOT/src" -type f \( \
    -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o \
    -name '*.h' -o -name '*.hpp' -o -name '*.hh' \
  \) \
    ! -path '*/third/*' \
    -print0
)

if ((${#FILES[@]} == 0)); then
  echo "no source files matched under $ROOT/src"
  exit 0
fi

"$CF" -i --style=file "${FILES[@]}"
echo "formatted ${#FILES[@]} file(s) with Google style (.clang-format)"
