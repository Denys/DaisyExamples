#!/usr/bin/env python3
"""Run the complete host qualification locally, retaining each command and exit status."""
import json
from pathlib import Path
import subprocess
import sys
from common import ROOT


def main():
    out = ROOT / "artifacts" / "integration"
    out.mkdir(parents=True, exist_ok=True)
    commands = [
        [sys.executable, "-S", "tools/validate_repo.py"],
        [sys.executable, "-S", "tools/generate_presets.py", "--check"],
        [sys.executable, "-S", "-m", "unittest", "discover", "-s", "tests/python", "-v"],
    ]
    for mode, flags in (
        ("host", ["-DCMAKE_BUILD_TYPE=Release"]),
        ("ubsan", ["-DCMAKE_BUILD_TYPE=Debug", "-DHPF_UBSAN_ONLY=ON"]),
        ("sanitize", ["-DCMAKE_BUILD_TYPE=Debug", "-DHPF_ENABLE_SANITIZERS=ON",
                      "-DCMAKE_CXX_COMPILER=clang++"]),
    ):
        directory = "build-" + mode
        commands.extend([
            ["cmake", "-S", ".", "-B", directory, *flags],
            ["cmake", "--build", directory, "--parallel", "2"],
            ["ctest", "--test-dir", directory, "--output-on-failure"],
        ])
    commands.extend([
        [str(ROOT / "build-host/render_scenarios"), str(out / "renders-a")],
        [str(ROOT / "build-host/render_scenarios"), str(out / "renders-b")],
        [sys.executable, "-S", "tools/validate_audio.py", str(out / "renders-a"),
         "--repeat", str(out / "renders-b")],
    ])
    results = []
    for index, command in enumerate(commands):
        log = out / f"host-qualification-{index:02d}.log"
        print("RUN " + " ".join(command), flush=True)
        with log.open("w") as stream:
            stream.write("$ " + " ".join(command) + "\n")
            stream.flush()
            result = subprocess.run(command, cwd=ROOT, stdout=stream,
                                    stderr=subprocess.STDOUT, timeout=1800)
            stream.write(f"\nexit={result.returncode}\n")
        results.append({"command": command, "exit": result.returncode, "log": log.name})
        (out / "host-qualification.json").write_text(json.dumps(results, indent=2) + "\n")
        if result.returncode:
            print(f"FAIL: {log}", flush=True)
            return result.returncode
    print("Host qualification PASS; hardware NOT_RUN.", flush=True)
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
