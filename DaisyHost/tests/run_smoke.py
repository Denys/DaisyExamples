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
PROCESS_QUERY_TIMEOUT_SECONDS = 30.0
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
    braids_scenario: Path
    harmoniqs_scenario: Path
    vasynth_scenario: Path
    polyosc_scenario: Path
    field_shell_scenario: Path
    field_native_controls_scenario: Path
    field_extended_surface_scenario: Path
    field_node_target_scenario: Path
    field_polyosc_surface_scenario: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run DaisyHost smoke checks.")
    parser.add_argument("--mode", choices=("standalone", "render", "all"), required=True)
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--source-dir", required=True)
    parser.add_argument("--config", default="Release")
    parser.add_argument(
        "--board",
        choices=("daisy_patch", "daisy_field"),
        help="Run standalone smoke for only this board.",
    )
    parser.add_argument(
        "--app",
        default="torus",
        help="Standalone app id to launch when --board is supplied.",
    )
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
        braids_scenario=source_dir / "training" / "examples" / "braids_smoke.json",
        harmoniqs_scenario=source_dir
        / "training"
        / "examples"
        / "harmoniqs_smoke.json",
        vasynth_scenario=source_dir
        / "training"
        / "examples"
        / "vasynth_smoke.json",
        polyosc_scenario=source_dir
        / "training"
        / "examples"
        / "polyosc_smoke.json",
        field_shell_scenario=source_dir
        / "training"
        / "examples"
        / "field_cloudseed_shell_smoke.json",
        field_native_controls_scenario=source_dir
        / "training"
        / "examples"
        / "field_vasynth_native_controls_smoke.json",
        field_extended_surface_scenario=source_dir
        / "training"
        / "examples"
        / "field_extended_surface_smoke.json",
        field_node_target_scenario=source_dir
        / "training"
        / "examples"
        / "field_node_target_surface_smoke.json",
        field_polyosc_surface_scenario=source_dir
        / "training"
        / "examples"
        / "field_polyosc_surface_smoke.json",
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
        timeout_seconds=PROCESS_QUERY_TIMEOUT_SECONDS,
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


def run_single_standalone_smoke(
    paths: SmokePaths, *, board_id: str, app_id: str, timeout_seconds: float
) -> None:
    require_existing_file(paths.standalone_executable, "Standalone executable")
    ensure_executable_not_running(paths.standalone_executable)
    ensure_executable_not_locked(paths.standalone_executable)

    command = [
        str(paths.standalone_executable),
        "--board",
        board_id,
        "--app",
        app_id,
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

    print(
        f"Standalone smoke passed for DaisyHost Patch.exe with board={board_id}, app={app_id}."
    )


def run_standalone_smoke(
    paths: SmokePaths, *, timeout_seconds: float, board_id: str | None = None, app_id: str = "torus"
) -> None:
    if board_id is not None:
        run_single_standalone_smoke(
            paths, board_id=board_id, app_id=app_id, timeout_seconds=timeout_seconds
        )
        return

    run_single_standalone_smoke(
        paths, board_id="daisy_patch", app_id="torus", timeout_seconds=timeout_seconds
    )
    run_single_standalone_smoke(
        paths, board_id="daisy_field", app_id="torus", timeout_seconds=timeout_seconds
    )


def verify_render_manifest(
    manifest_path: Path, *, expected_app_id: str, expected_board_id: str | None = None
) -> str:
    require_existing_file(manifest_path, "Render manifest")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    actual_app_id = manifest.get("appId")
    if actual_app_id != expected_app_id:
        raise SmokeFailure(
            f"Manifest appId mismatch for {manifest_path}: expected {expected_app_id}, got {actual_app_id}"
        )
    if expected_board_id is not None:
        actual_board_id = manifest.get("boardId")
        if actual_board_id != expected_board_id:
            raise SmokeFailure(
                f"Manifest boardId mismatch for {manifest_path}: expected {expected_board_id}, got {actual_board_id}"
            )
    checksum = manifest.get("audioChecksum")
    if not isinstance(checksum, str) or not checksum:
        raise SmokeFailure(f"Manifest audioChecksum missing or invalid in {manifest_path}")
    return checksum


def verify_manifest_parameter_values(
    manifest_path: Path, expected_values: dict[str, float], *, tolerance: float = 0.0001
) -> None:
    require_existing_file(manifest_path, "Render manifest")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    final_values = manifest.get("finalParameterValues")
    if not isinstance(final_values, dict):
        raise SmokeFailure(f"Manifest finalParameterValues missing in {manifest_path}")
    for parameter_id, expected_value in expected_values.items():
        actual_value = final_values.get(parameter_id)
        if not isinstance(actual_value, (int, float)):
            raise SmokeFailure(
                f"Manifest finalParameterValues missing {parameter_id} in {manifest_path}"
            )
        if abs(float(actual_value) - expected_value) > tolerance:
            raise SmokeFailure(
                f"Manifest final value mismatch for {parameter_id}: "
                f"expected {expected_value}, got {actual_value}"
            )


def verify_manifest_field_surface(manifest_path: Path, *, tolerance: float = 0.0001) -> None:
    require_existing_file(manifest_path, "Render manifest")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    surface = manifest.get("fieldSurface")
    if not isinstance(surface, dict):
        raise SmokeFailure(f"Manifest fieldSurface missing in {manifest_path}")

    cv_outputs = surface.get("cvOutputs")
    switches = surface.get("switches")
    leds = surface.get("leds")
    if not isinstance(cv_outputs, list) or len(cv_outputs) < 2:
        raise SmokeFailure(f"Manifest fieldSurface.cvOutputs missing in {manifest_path}")
    if not isinstance(switches, list) or len(switches) < 2:
        raise SmokeFailure(f"Manifest fieldSurface.switches missing in {manifest_path}")
    if not isinstance(leds, list) or len(leds) < 20:
        raise SmokeFailure(f"Manifest fieldSurface.leds missing in {manifest_path}")

    expected_cv = [
        ("node0/port/field_cv_out_1", 0.82, 4.10),
        ("node0/port/field_cv_out_2", 0.44, 2.20),
    ]
    for index, (expected_id, expected_normalized, expected_volts) in enumerate(expected_cv):
        item = cv_outputs[index]
        if item.get("id") != expected_id or item.get("available") is not True:
            raise SmokeFailure(f"Field CV OUT {index + 1} manifest evidence is invalid")
        if abs(float(item.get("normalizedValue", -1.0)) - expected_normalized) > tolerance:
            raise SmokeFailure(f"Field CV OUT {index + 1} normalized value mismatch")
        if abs(float(item.get("volts", -1.0)) - expected_volts) > tolerance:
            raise SmokeFailure(f"Field CV OUT {index + 1} voltage mismatch")

    if switches[0].get("id") != "node0/control/field_sw_1" or switches[0].get("pressed") is not True:
        raise SmokeFailure("Field SW1 manifest evidence is invalid")
    if switches[0].get("detailLabel") != "Back":
        raise SmokeFailure("Field SW1 navigation label evidence is invalid")
    if switches[1].get("id") != "node0/control/field_sw_2" or switches[1].get("pressed") is not True:
        raise SmokeFailure("Field SW2 manifest evidence is invalid")
    if switches[1].get("detailLabel") != "Forward":
        raise SmokeFailure("Field SW2 navigation label evidence is invalid")

    expected_leds = {
        "node0/led/field_key_a_1": 1.0,
        "node0/led/field_sw_1": 1.0,
        "node0/led/field_sw_2": 1.0,
        "node0/led/field_gate_in": 1.0,
    }
    led_map = {item.get("id"): item for item in leds if isinstance(item, dict)}
    for led_id, expected_value in expected_leds.items():
        item = led_map.get(led_id)
        if item is None:
            raise SmokeFailure(f"Field LED manifest evidence missing for {led_id}")
        if abs(float(item.get("normalizedValue", -1.0)) - expected_value) > tolerance:
            raise SmokeFailure(f"Field LED value mismatch for {led_id}")


def verify_manifest_field_node_target(manifest_path: Path, *, tolerance: float = 0.0001) -> None:
    require_existing_file(manifest_path, "Render manifest")
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("boardId") != "daisy_field":
        raise SmokeFailure("Field node-target manifest boardId evidence is invalid")
    if manifest.get("selectedNodeId") != "node1":
        raise SmokeFailure("Field node-target manifest selectedNodeId evidence is invalid")

    surface = manifest.get("fieldSurface")
    if not isinstance(surface, dict):
        raise SmokeFailure("Field node-target manifest fieldSurface missing")
    cv_outputs = surface.get("cvOutputs")
    if not isinstance(cv_outputs, list) or len(cv_outputs) < 2:
        raise SmokeFailure("Field node-target CV OUT evidence missing")
    expected_cv = [
        ("node1/port/field_cv_out_1", 0.82, 4.10),
        ("node1/port/field_cv_out_2", 0.44, 2.20),
    ]
    for index, (expected_id, expected_normalized, expected_volts) in enumerate(expected_cv):
        item = cv_outputs[index]
        if item.get("id") != expected_id or item.get("available") is not True:
            raise SmokeFailure(f"Field node-target CV OUT {index + 1} id evidence is invalid")
        if abs(float(item.get("normalizedValue", -1.0)) - expected_normalized) > tolerance:
            raise SmokeFailure(f"Field node-target CV OUT {index + 1} normalized mismatch")
        if abs(float(item.get("volts", -1.0)) - expected_volts) > tolerance:
            raise SmokeFailure(f"Field node-target CV OUT {index + 1} volts mismatch")

    nodes = manifest.get("nodes")
    if not isinstance(nodes, list) or len(nodes) < 2:
        raise SmokeFailure("Field node-target nodes evidence missing")
    node1 = next((node for node in nodes if node.get("nodeId") == "node1"), None)
    if not isinstance(node1, dict):
        raise SmokeFailure("Field node-target node1 evidence missing")
    final_values = node1.get("finalParameterValues")
    if not isinstance(final_values, dict):
        raise SmokeFailure("Field node-target node1 finalParameterValues missing")
    actual = final_values.get("node1/param/filter_cutoff")
    if not isinstance(actual, (int, float)) or abs(float(actual) - 0.82) > tolerance:
        raise SmokeFailure("Field node-target node1 parameter evidence mismatch")


def run_single_render(
    render_executable: Path,
    scenario_path: Path,
    *,
    output_dir: Path,
    expected_app_id: str,
    expected_board_id: str | None = None,
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
    return verify_render_manifest(
        manifest_path,
        expected_app_id=expected_app_id,
        expected_board_id=expected_board_id,
    )


def run_render_smoke(paths: SmokePaths, *, timeout_seconds: float) -> None:
    require_existing_file(paths.render_executable, "Render executable")
    require_existing_file(paths.multidelay_scenario, "MultiDelay smoke scenario")
    require_existing_file(paths.torus_scenario, "Torus smoke scenario")
    require_existing_file(paths.cloudseed_scenario, "CloudSeed smoke scenario")
    require_existing_file(paths.braids_scenario, "Braids smoke scenario")
    require_existing_file(paths.harmoniqs_scenario, "Harmoniqs smoke scenario")
    require_existing_file(paths.vasynth_scenario, "VA Synth smoke scenario")
    require_existing_file(paths.polyosc_scenario, "PolyOsc smoke scenario")
    require_existing_file(paths.field_shell_scenario, "Daisy Field shell smoke scenario")
    require_existing_file(
        paths.field_native_controls_scenario,
        "Daisy Field native controls smoke scenario",
    )
    require_existing_file(
        paths.field_extended_surface_scenario,
        "Daisy Field extended surface smoke scenario",
    )
    require_existing_file(
        paths.field_node_target_scenario,
        "Daisy Field selected-node surface smoke scenario",
    )
    require_existing_file(
        paths.field_polyosc_surface_scenario,
        "Daisy Field PolyOsc surface smoke scenario",
    )

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
        run_single_render(
            paths.render_executable,
            paths.braids_scenario,
            output_dir=workspace / "braids_run",
            expected_app_id="braids",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        run_single_render(
            paths.render_executable,
            paths.harmoniqs_scenario,
            output_dir=workspace / "harmoniqs_run",
            expected_app_id="harmoniqs",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        run_single_render(
            paths.render_executable,
            paths.vasynth_scenario,
            output_dir=workspace / "vasynth_run",
            expected_app_id="vasynth",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        run_single_render(
            paths.render_executable,
            paths.polyosc_scenario,
            output_dir=workspace / "polyosc_run",
            expected_app_id="polyosc",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        run_single_render(
            paths.render_executable,
            paths.field_shell_scenario,
            output_dir=workspace / "field_cloudseed_shell_run",
            expected_app_id="cloudseed",
            expected_board_id="daisy_field",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        native_controls_dir = workspace / "field_vasynth_native_controls_run"
        run_single_render(
            paths.render_executable,
            paths.field_native_controls_scenario,
            output_dir=native_controls_dir,
            expected_app_id="vasynth",
            expected_board_id="daisy_field",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        verify_manifest_parameter_values(
            native_controls_dir / "manifest.json",
            {
                "node0/param/filter_cutoff": 0.82,
                "node0/param/resonance": 0.44,
                "node0/param/filter_env_amount": 0.33,
                "node0/param/level": 0.77,
            },
        )
        extended_surface_dir = workspace / "field_extended_surface_run"
        run_single_render(
            paths.render_executable,
            paths.field_extended_surface_scenario,
            output_dir=extended_surface_dir,
            expected_app_id="vasynth",
            expected_board_id="daisy_field",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        verify_manifest_field_surface(extended_surface_dir / "manifest.json")
        node_target_dir = workspace / "field_node_target_surface_run"
        run_single_render(
            paths.render_executable,
            paths.field_node_target_scenario,
            output_dir=node_target_dir,
            expected_app_id="vasynth",
            expected_board_id="daisy_field",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        verify_manifest_field_node_target(node_target_dir / "manifest.json")
        field_polyosc_dir = workspace / "field_polyosc_surface_run"
        run_single_render(
            paths.render_executable,
            paths.field_polyosc_surface_scenario,
            output_dir=field_polyosc_dir,
            expected_app_id="polyosc",
            expected_board_id="daisy_field",
            timeout_seconds=timeout_seconds,
            cwd=paths.source_dir,
        )
        verify_manifest_parameter_values(
            field_polyosc_dir / "manifest.json",
            {"node0/param/waveform": 1.0},
        )
        success = True
    except SmokeFailure as exc:
        raise SmokeFailure(f"{exc}\nRender smoke outputs preserved at: {workspace}") from exc
    finally:
        if success:
            shutil.rmtree(workspace, ignore_errors=True)

    print(
        "Render smoke passed for MultiDelay, Torus, CloudSeed, Braids, Harmoniqs, VA Synth, PolyOsc, Daisy Field shell, Daisy Field native controls, Daisy Field extended surface, Daisy Field selected-node surface, and Daisy Field PolyOsc surface scenarios."
    )


def main() -> int:
    args = parse_args()
    paths = resolve_paths(args)
    try:
        if args.mode in ("standalone", "all"):
            run_standalone_smoke(
                paths,
                timeout_seconds=args.timeout_seconds,
                board_id=args.board,
                app_id=args.app,
            )
        if args.mode in ("render", "all"):
            run_render_smoke(paths, timeout_seconds=args.timeout_seconds)
    except SmokeFailure as exc:
        print(f"DaisyHost smoke failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
