#!/usr/bin/env python3
"""Format C/C++ sources with clang-format using repo .clang-format."""

import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List


FORMAT_DIRS = ("daeml", "server", "test")
SOURCE_SUFFIXES = {".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh"}


def collect_source_files(root: Path) -> List[str]:
    files: List[str] = []

    for relative_dir in FORMAT_DIRS:
        base_dir = root / relative_dir
        if not base_dir.is_dir():
            continue

        for current_root, dirnames, filenames in os.walk(base_dir):
            # Skip third-party or generated trees to avoid unrelated diffs.
            dirnames[:] = [dirname for dirname in dirnames if dirname != "third"]

            for filename in filenames:
                file_path = Path(current_root) / filename
                if file_path.suffix in SOURCE_SUFFIXES:
                    files.append(str(file_path))

    files.sort()
    return files


def main() -> int:
    root = Path(__file__).resolve().parent
    clang_format = os.environ.get("CLANG_FORMAT", "clang-format")

    if shutil.which(clang_format) is None:
        print(
            f"error: '{clang_format}' not found. Install LLVM clang-format or set CLANG_FORMAT.",
            file=sys.stderr,
        )
        return 1

    files = collect_source_files(root)
    if not files:
        print(f"no source files matched under {' '.join(FORMAT_DIRS)}")
        return 0

    subprocess.run([clang_format, "-i", "--style=file", *files], check=True)
    print(f"formatted {len(files)} file(s) with Google style (.clang-format)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
