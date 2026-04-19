#!/usr/bin/env python3
from __future__ import annotations

import argparse
import copy
import itertools
import json
import subprocess
from pathlib import Path
from typing import Any, Dict, Iterable, List, Tuple


def load_json(path: Path) -> Dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def slugify(value: Any) -> str:
    text = str(value).strip().lower().replace(" ", "_").replace("-", "_")
    return "".join(ch for ch in text if ch.isalnum() or ch == "_") or "value"


def default_renderer_path(script_dir: Path) -> Path:
    return (script_dir.parent / "build" / "Release" / "DaisyHostRender.exe").resolve()


def load_base_scenario(config: Dict[str, Any], config_path: Path) -> Dict[str, Any]:
    if "baseScenario" in config:
        return copy.deepcopy(config["baseScenario"])

    if "baseScenarioPath" in config:
        scenario_path = (config_path.parent / config["baseScenarioPath"]).resolve()
        return load_json(scenario_path)

    raise ValueError("Dataset job config must define baseScenario or baseScenarioPath")


def apply_sweep(scenario: Dict[str, Any], sweep: Dict[str, Any], value: Any) -> str:
    kind = sweep["kind"]

    if kind == "seed":
        scenario["seed"] = value
        return f"seed_{slugify(value)}"

    if kind == "parameter":
        parameter_id = sweep["parameterId"]
        scenario.setdefault("initialParameterValues", {})[parameter_id] = value
        return f"param_{slugify(parameter_id.rsplit('/', 1)[-1])}_{slugify(value)}"

    if kind == "audio_input_mode":
        scenario.setdefault("audioInput", {})["mode"] = value
        return f"mode_{slugify(value)}"

    if kind == "audio_input_level":
        scenario.setdefault("audioInput", {})["level"] = value
        return f"level_{slugify(value)}"

    if kind == "audio_input_frequency":
        scenario.setdefault("audioInput", {})["frequencyHz"] = value
        return f"freq_{slugify(value)}"

    raise ValueError(f"Unsupported sweep kind: {kind}")


def expand_runs(base_scenario: Dict[str, Any], sweeps: List[Dict[str, Any]]) -> Iterable[Tuple[str, Dict[str, Any]]]:
    if not sweeps:
        yield "run_0000", copy.deepcopy(base_scenario)
        return

    value_lists = [sweep["values"] for sweep in sweeps]
    for index, combination in enumerate(itertools.product(*value_lists)):
        scenario = copy.deepcopy(base_scenario)
        labels: List[str] = []
        for sweep, value in zip(sweeps, combination):
            labels.append(apply_sweep(scenario, sweep, value))
        run_name = f"run_{index:04d}"
        if labels:
            run_name += "__" + "__".join(labels)
        yield run_name, scenario


def main() -> int:
    parser = argparse.ArgumentParser(description="Expand dataset sweeps and invoke DaisyHostRender.")
    parser.add_argument("job_config", type=Path, help="Path to the dataset job JSON file")
    parser.add_argument(
        "--renderer",
        type=Path,
        default=None,
        help="Optional explicit path to DaisyHostRender.exe",
    )
    args = parser.parse_args()

    config_path = args.job_config.resolve()
    config = load_json(config_path)
    script_dir = Path(__file__).resolve().parent

    renderer_path = (args.renderer.resolve() if args.renderer is not None else default_renderer_path(script_dir))
    if not renderer_path.exists():
        raise FileNotFoundError(f"Renderer executable not found: {renderer_path}")

    base_scenario = load_base_scenario(config, config_path)
    sweeps = config.get("sweeps", [])
    output_dir_value = config.get("outputDir", config.get("jobName", config_path.stem))
    output_dir_path = Path(output_dir_value)
    if not output_dir_path.is_absolute():
        output_dir_path = config_path.parent / output_dir_path
    output_dir = output_dir_path.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    dataset_index: Dict[str, Any] = {
        "jobName": config.get("jobName", config_path.stem),
        "rendererPath": str(renderer_path),
        "runs": [],
    }

    for run_name, scenario in expand_runs(base_scenario, sweeps):
        run_dir = output_dir / run_name
        run_dir.mkdir(parents=True, exist_ok=True)

        scenario_path = run_dir / "scenario.json"
        with scenario_path.open("w", encoding="utf-8") as handle:
            json.dump(scenario, handle, indent=2)

        command = [str(renderer_path), str(scenario_path), "--output-dir", str(run_dir)]
        subprocess.run(command, check=True)

        manifest_path = run_dir / "manifest.json"
        audio_path = run_dir / "audio.wav"
        dataset_index["runs"].append(
            {
                "name": run_name,
                "scenarioPath": str(scenario_path),
                "manifestPath": str(manifest_path),
                "audioPath": str(audio_path),
            }
        )

    index_path = output_dir / "dataset_index.json"
    with index_path.open("w", encoding="utf-8") as handle:
        json.dump(dataset_index, handle, indent=2)

    print(f"Wrote dataset index to {index_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
