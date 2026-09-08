#!/usr/bin/env python3
"""Verify a real libDaisy checkout, including recursive gitlink state. Does not fetch."""
import argparse
import json
from pathlib import Path
import re
import sys
import subprocess
from common import ROOT, git

def verify(directory: Path, lock: dict) -> dict:
    dependency = lock["dependencies"][0]
    expected = dependency["ref"]
    if not re.fullmatch(r"[0-9a-f]{40}", expected):
        raise ValueError("libDaisy pin must be a full commit SHA")
    if not directory.is_dir():
        raise ValueError(f"libDaisy checkout missing: {directory}")
    actual = git(directory, "rev-parse", "HEAD")
    if actual != expected:
        raise ValueError(f"libDaisy HEAD mismatch: expected {expected}, found {actual}")
    status = git(directory, "status", "--porcelain", "--untracked-files=no", "--ignore-submodules=none")
    if status:
        raise ValueError(f"libDaisy has tracked modifications: {status}")
    license_blob = git(directory, "rev-parse", "HEAD:LICENSE")
    if license_blob != dependency["license_blob_sha1"]:
        raise ValueError("libDaisy LICENSE does not match the recorded blob")
    submodules = git(directory, "submodule", "status", "--recursive")
    # git() strips leading spaces. Inspect each SHA and explicitly reject '+', '-', 'U'.
    for line in submodules.splitlines():
        if line and line[0] in "+-U":
            raise ValueError(f"uninitialized or mismatched submodule: {line}")
        if not re.match(r"\s*[0-9a-f]{40} ", line):
            raise ValueError(f"unrecognized submodule record: {line}")
    # status above checks parent tracked submodule dirtiness; recursively inspect nested tracked files.
    dirty = git(directory, "submodule", "foreach", "--quiet", "--recursive",
                "git status --porcelain --untracked-files=no --ignore-submodules=none")
    if dirty:
        raise ValueError(f"dirty nested dependency: {dirty}")
    for required in ("core/Makefile", "src/daisy_field.h", "src/util/CpuLoadMeter.h"):
        if not (directory / required).is_file():
            raise ValueError(f"dependency file missing: {required}")
    return {"head": actual, "license_blob_sha1": license_blob,
            "recursive_gitlinks": submodules.splitlines(), "tracked_clean": True}

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("libdaisy", type=Path)
    args = parser.parse_args()
    try:
        result = verify(args.libdaisy.resolve(), json.loads((ROOT / "deps.lock.json").read_text()))
        print(json.dumps(result, indent=2))
        return 0
    except (ValueError, KeyError, OSError, subprocess.TimeoutExpired) as e:
        print(f"dependency verification: FAIL: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
