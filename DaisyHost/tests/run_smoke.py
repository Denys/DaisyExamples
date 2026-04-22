#!/usr/bin/env python3
"""Run DaisyHost standalone and render smoke checks."""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
import time
import uuid
from dataclasses import dataclass
from pathlib import Path


STANDALONE_WARMUP_SECONDS = 5.0
PROCESS_EXIT_TIMEOUT_SECONDS = 10.0
CREATE_NO_WINDOW = getattr(subprocess, "CREATE_NO_WINDOW", 0)


class SmokeFailure(RuntimeError):
    """Raised when a smoke check fails."""


@dataclass(frozen=True)
class SmokePaths:
    build_dir: Path
    source_dir: Path
    config: str
    standalone_executable: Path
    render_executable: Path
    multidelay_scenario: Path
    torus_scenario: Path
    cloudseed_scenario: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run DaisyHost smoke checks.")
    parser.add_argument("--mode", choices=("standalone", "render", "all"), required=True)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--source-dir", required=True)
    parser.add_argument("--config", default="Release")
    parser.add_argument("--timeout-seconds", type=float, default=60.0)
    return parser.parse_args()


def resolve_paths(args: argparse.Namespace) -> SmokePaths:
    build_dir = Path(args.build_dir).resolve()
    source_dir = Path(args.source_dir).resolve()
    config = args.config or "Release"
    return SmokePaths(
        build_dir=build_dir,
        source_dir=source_dir,
        config=config,
        standalone_executable=build_dir
        / "DaisyHostPatch_artefacts"
        / config
        / "Standalone"
        / "DaisyHost Patch.exe",
        render_executable=build_dir / config / "DaisyHostRender.exe",
        multidelay_scenario=source_dir
        / "training"
        / "examples"
        / "multidelay_smoke.json",
        torus_scenario=source_dir / "training" / "examples" / "torus_smoke.json",
        cloudseed_scenario=source_dir
        / "training"
        / "examples"
        / "cloudseed_smoke.json",
    )


def require_existing_file(path: Path, description: str) -> None:
    if not path.exists():
        raise SmokeFailure(f"{description} not found: {path}")
    if not path.is_file():
        raise SmokeFailure(f"{description} is not a file: {path}")


def format_command(command: list[str]) -> str:
    return " ".join(f'"{part}"' if " " in part else part for part in command)


def run_command(command: list[str], *, timeout_seconds: float, cwd: Path | None = None) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            command,
            cwd=str(cwd) if cwd is not None else None,
            text=True,
            capture_output=True,
            timeout=timeout_seconds,
            check=False,
            creationflags=CREATE_NO_WINDOW,
        )
    except subprocess.TimeoutExpired as exc:
        raise SmokeFailure(
            f"Command timed out after {timeout_seconds:.1f}s: {format_command(command)}"
        ) from exc


def query_processes_for_path(executable_path: Path) -> list[dict[str, object]]:
    target = str(executable_path).replace("'", "''")
    process_name = executable_path.stem.replace("'", "''")
    script = (
        f"$target = '{target}'; "
        f"$processName = '{process_name}'; "
        "$matches = @(Get-Process -Name $processName -ErrorAction SilentlyContinue | "
        "Where-Object { try { $_.Path -eq $target } catch { $false } } | "
        "Select-Object Id, ProcessName, Path); "
        "if($matches.Count -eq 0) { '[]' } else { $matches | ConvertTo-Json -Compress }"
    )
    result = run_command(
        ["powershell.exe", "-NoProfile", "-Command", script],
        timeout_seconds=PROCESS_EXIT_TIMEOUT_SECONDS,
    )
    if result.returncode != 0:
        raise SmokeFailure(
            "Failed to query running processes for standalone smoke:\n"
            f"{result.stderr.strip() or result.stdout.strip()}"
        )
    text = result.stdout.strip() or "[]"
    decoded = json.loads(text)
    if isinstance(decoded, dict):
        return [decoded]
    return decoded


def ensure_executable_not_running(executable_path: Path) -> None:
    matches = query_processes_for_path(executable_path)
    if matches:
        ids = ", ".join(str(match.get("Id", "?")) for match in matches)
        raise SmokeFailure(
            f"Stale standalone process already running for {executable_path} (PID(s): {ids})"
        )


def ensure_executable_not_locked(executable_path: Path) -> None:
    try:
        with executable_path.open("rb+"):
            pass
    except OSError as exc:
        raise SmokeFailure(f"Executable appears locked before launch: {executable_path}: {exc}") from exc


def terminate_process(process: subprocess.Popen[bytes], *, label: str) -> None:
    if process.poll() is not None:
        return

    process.terminate()
    try:
        process.wait(timeout=PROCESS_EXIT_TIMEOUT_SECONDS)
    except subprocess.TimeoutExpired:
        process.kill()
        try:
            process.wait(timeout=PROCESS_EXIT_TIMEOUT_SECONDS)
        except subprocess.TimeoutExpired as exc:
            raise SmokeFailure(f"{label} did not terminate cleanly") from exc


def run_standalone_smoke(paths: SmokePaths, *, timeout_seconds: float) -> None:
    require_existing_file(paths.standalone_executable, "Standalone executable")
    ensure_executable_not_running(paths.standalone_executable)
    ensure_executable_not_locked(paths.standalone_executable)

    command = [
        str(paths.standalone_executable),
        "--board",
        "daisy_patch",
        "--app",
        "torus",
    ]
    process: subprocess.Popen[bytes] | None = None
    try:
        process = subprocess.Popen(
            command,
            cwd=str(paths.standalone_executable.parent),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            creationflags=CREATE_NO_WINDOW,
        )
        deadline = time.monotonic() + STANDALONE_WARMUP_SECONDS
        while time.monotonic() < deadline:
            return_code = process.poll()
            if return_code is not None:
                raise SmokeFailure(
                    "Standalone executable exited before the warmup window "
                    f"with code {return_code}: {format_command(command)}"
                )
            time.sleep(0.1)

        terminate_process(process, label="Standalone smoke process")
        if query_processes_for_path(paths.standalone_executable):
            raise SmokeFailure(
                "Standalone process was terminated but an instance is still running "
                f"for {paths.standalone_executable}"
            )
    except OSError as exc:
        raise SmokeFailure(f"Failed to launch standalone executable: {exc}") from exc
    finally:
        if process is not None:
            terminate_process(process, label="Standalone smoke cleanup")

    print("Standalone smoke passed for DaisyHost Patch.exe with app=torus.")


def verify_render_manifest(manifest_path: Path, *, expected_app_id: str) -> str:
    require_existing_file(manifest_path, "Render manifest")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    actual_app_id = manifest.get("appId")
    if actual_app_id != expected_app_id:
        raise SmokeFailure(
            f"Manifest appId mismatch for {manifest_path}: expected {expected_app_id}, got {actual_app_id}"
        )
    checksum = manifest.get("audioChecksum")
    if not isinstance(checksum, str) or not checksum:
        raise SmokeFailure(f"Manifest audioChecksum missing or invalid in {manifest_path}")
    return checksum


def run_single_render(
    render_executable: Path,
    scenario_path: Path,
    *,
    output_dir: Path,
    expected_app_id: str,
    timeout_seconds: float,
    cwd: Path,
) -> str:
    output_dir.mkdir(parents=True, exist_ok=True)
    command = [
        str(render_executable),
        str(scenario_path),
        "--output-dir",
        str(output_dir),
    ]
    result = run_command(command, timeout_seconds=timeout_seconds, cwd=cwd)
    if result.returncode != 0:
        raise SmokeFailure(
            f"Render command failed ({result.returncode}): {format_command(command)}\n"
            f"stdout:\n{result.stdout.strip()}\n"
            f"stderr:\n{result.stderr.strip()}"
        )

    audio_path = output_dir / "audio.wav"
    manifest_path = output_dir / "manifest.json"
    require_existing_file(audio_path, "Rendered audio output")
    return verify_render_manifest(manifest_path, expected_app_id=expected_app_id)


def run_render_smoke(paths: SmokePaths, *, timeout_seconds: float) -> None:
    require_existing_file(paths.render_executable, "Render executable")
    require_existing_file(paths.multidelay_scenario, "MultiDelay smoke scenario")
    require_existing_file(paths.torus_scenario, "Torus smoke scenario")
    require_existing_file(paths.cloudseed_scenario, "CloudSeed smoke scenario")

    workspace = paths.build_dir / "smoke" / f"render_{uuid.uuid4().hex[:8]}"
    workspace.mkdir(parents=True, exist_ok=True)
    success = False
    try:
        first_multidelay_checksum = run_single_render(
            paths.render_executable,
            paths.multidelay_scenario,
            output_dir=workspace / "multidelay_run_1",
            expected_app_id="multidelay",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        second_multidelay_checksum = run_single_render(
            paths.render_executable,
            paths.multidelay_scenario,
            output_dir=workspace / "multidelay_run_2",
            expected_app_id="multidelay",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        if first_multidelay_checksum != second_multidelay_checksum:
            raise SmokeFailure(
                "MultiDelay render checksum drift detected between repeated smoke runs: "
                f"{first_multidelay_checksum} vs {second_multidelay_checksum}"
            )

        run_single_render(
            paths.render_executable,
            paths.torus_scenario,
            output_dir=workspace / "torus_run",
            expected_app_id="torus",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        run_single_render(
            paths.render_executable,
            paths.cloudseed_scenario,
            output_dir=workspace / "cloudseed_run",
            expected_app_id="cloudseed",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        success = True
    except SmokeFailure as exc:
        raise SmokeFailure(f"{exc}\nRender smoke outputs preserved at: {workspace}") from exc
    finally:
        if success:
            shutil.rmtree(workspace, ignore_errors=True)

    print("Render smoke passed for MultiDelay, Torus, and CloudSeed scenarios.")


def main() -> int:
    args = parse_args()
    paths = resolve_paths(args)
    try:
        if args.mode in ("standalone", "all"):
            run_standalone_smoke(paths, timeout_seconds=args.timeout_seconds)
        if args.mode in ("render", "all"):
            run_render_smoke(paths, timeout_seconds=args.timeout_seconds)
    except SmokeFailure as exc:
        print(f"DaisyHost smoke failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
