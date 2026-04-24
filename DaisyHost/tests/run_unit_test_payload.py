#!/usr/bin/env python3
"""Launch a DaisyHost unit-test payload from a fresh temporary binary copy."""

from __future__ import annotations

import shutil
import subprocess
import sys
import uuid
from pathlib import Path


def find_build_dir(executable_path: Path) -> Path:
    for parent in executable_path.parents:
        if (parent / "CMakeCache.txt").is_file():
            return parent
    raise FileNotFoundError(
        f"Could not locate CMake build root above {executable_path}"
    )


def main() -> int:
    if len(sys.argv) < 2:
        print("Expected original unit-test executable path as argv[1].", file=sys.stderr)
        return 2

    original_executable = Path(sys.argv[1]).resolve()
    test_args = sys.argv[2:]
    if not original_executable.is_file():
        print(f"Unit-test payload not found: {original_executable}", file=sys.stderr)
        return 3

    config_name = original_executable.parent.name
    build_dir = find_build_dir(original_executable)

    run_dir = build_dir / "unit_test_run" / config_name
    run_dir.mkdir(parents=True, exist_ok=True)
    temp_executable = run_dir / f"DaisyHostUnitTests_{uuid.uuid4().hex}.bin"

    try:
        shutil.copyfile(original_executable, temp_executable)
        completed = subprocess.run([str(temp_executable), *test_args], cwd=run_dir)
        return completed.returncode
    finally:
        try:
            temp_executable.unlink()
        except OSError:
            pass


if __name__ == "__main__":
    raise SystemExit(main())
