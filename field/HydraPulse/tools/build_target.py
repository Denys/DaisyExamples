#!/usr/bin/env python3
"""Build both real ARM images after dependency checks. Never flashes hardware."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
from common import ROOT, git, runtime_sources, manifest_digest, sha256
from verify_dependencies import verify

TOOLS = ("git", "make", "arm-none-eabi-gcc", "arm-none-eabi-g++",
         "arm-none-eabi-objcopy", "arm-none-eabi-size", "arm-none-eabi-readelf")

def require_tools() -> dict[str, str]:
    found = {name: shutil.which(name) for name in TOOLS}
    missing = [name for name, path in found.items() if not path]
    if missing:
        raise ValueError("missing tools: " + ", ".join(missing) +
                         "; use the documented ARM environment, not the adapter stub")
    return found

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--libdaisy", type=Path, required=True)
    parser.add_argument("--app-type", choices=("BOOT_NONE", "BOOT_SRAM", "BOOT_QSPI"), default="BOOT_NONE")
    parser.add_argument("--jobs", type=int, default=2)
    parser.add_argument("--output", type=Path, default=ROOT / "artifacts")
    args = parser.parse_args()
    try:
        paths = require_tools()
        if not 1 <= args.jobs <= 32:
            raise ValueError("--jobs must be between 1 and 32")
        lib = args.libdaisy.resolve()
        if any(c.isspace() for c in str(ROOT) + str(lib)):
            raise ValueError("upstream Makefiles require paths without spaces; use a dedicated WSL checkout")
        lock = json.loads((ROOT / "deps.lock.json").read_text())
        dependency = verify(lib, lock)
        sources = runtime_sources()
        source_id = manifest_digest(sources)
        output = args.output.resolve() / source_id[:12] / args.app_type
        output.mkdir(parents=True, exist_ok=True)
        transcript = []
        def run(command: list[str], timeout: int = 1800) -> str:
            r = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                               timeout=timeout, check=False)
            transcript.append("$ " + " ".join(command) + "\n" + r.stdout + r.stderr +
                              f"\nexit={r.returncode}\n")
            (output / "build.log").write_text("\n".join(transcript))
            if r.returncode:
                raise ValueError(f"build command failed ({r.returncode}); see {output / 'build.log'}")
            return r.stdout
        compiler = run([paths["arm-none-eabi-g++"], "--version"], 30)
        gcc_path = str(Path(paths["arm-none-eabi-g++"]).parent)
        if any(c.isspace() for c in gcc_path):
            raise ValueError("compiler path must not contain whitespace for these Makefiles")
        run([sys.executable, "-S", str(ROOT / "tools/generate_presets.py"), "--check"], 30)
        # Force object recompilation without remaking existing directory targets.
        # The pinned upstream directory recipe uses mkdir without -p.
        (lib / "build").mkdir(exist_ok=True)
        run(["make", "-C", str(lib), "-B", "-o", "build", f"-j{args.jobs}", f"GCC_PATH={gcc_path}",
             "OPT=-O3 -fno-short-enums -fno-fast-math -ffp-contract=off"])
        images = {}
        for app, name in (("truth", "HydraPulseTruth"), ("beat", "HydraPulseBeat")):
            (ROOT / f"build-{app}").mkdir(exist_ok=True)
            run(["make", "-C", str(ROOT), "-B", "-o", f"build-{app}", f"-j{args.jobs}", f"APP={app}",
                 f"APP_TYPE={args.app_type}", f"LIBDAISY_DIR={lib}", f"GCC_PATH={gcc_path}",
                 f"BUILD_ID={source_id[:12]}"])
            binary = ROOT / f"build-{app}" / f"{name}.bin"
            elf = binary.with_suffix(".elf")
            mapfile = binary.with_suffix(".map")
            if not all(p.is_file() and p.stat().st_size for p in (binary, elf, mapfile)):
                raise ValueError(f"{app}: missing or empty BIN/ELF/MAP")
            header = run([paths["arm-none-eabi-readelf"], "-h", str(elf)], 30)
            if not any("Machine:" in line and line.strip().endswith("ARM") for line in header.splitlines()):
                raise ValueError("output is not an ARM ELF")
            if args.app_type == "BOOT_NONE" and binary.stat().st_size > 128 * 1024:
                raise ValueError("BOOT_NONE image exceeds the project's 128 KiB qualification policy; "
                                 "review a bootloader route, never flash by guessing")
            run([paths["arm-none-eabi-size"], "-A", str(elf)], 30)
            record = {}
            for p in (binary, elf, mapfile):
                dest = output / p.name
                shutil.copy2(p, dest)
                record[p.suffix] = {"file": dest.name, "bytes": dest.stat().st_size, "sha256": sha256(dest)}
            images[app] = record
        # Catch an editor or another job changing inputs underneath the build.
        if runtime_sources() != sources:
            raise ValueError("runtime sources changed during build; artifact qualification rejected")
        verify(lib, lock)
        try:
            parent = git(ROOT, "rev-parse", "HEAD")
            dirty = git(ROOT, "status", "--porcelain", "--ignore-submodules=all", "--", ".")
        except ValueError:
            parent, dirty = None, "not in a Git checkout"
        report = {"status": "VERIFIED", "claim": "ARM build, NOT hardware qualification",
                  "source_sha256": source_id, "source_files": sources, "compiler": compiler,
                  "tools": paths, "dependency": dependency, "app_type": args.app_type,
                  "abi": "default-width enums in application and library",
                  "parent_commit": parent, "workspace_status": dirty, "images": images}
        (output / "build_manifest.json").write_text(json.dumps(report, indent=2) + "\n")
        print(f"ARM build PASS: {output}\nHardware test NOT_RUN. No flash operation performed.")
        return 0
    except (ValueError, KeyError, OSError, subprocess.TimeoutExpired) as e:
        print(f"ARM build: FAIL: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
