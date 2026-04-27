#!/usr/bin/env python3
"""Suggest the next DaisyHost work package from the local workstream tracker."""

from __future__ import annotations

import argparse
import dataclasses
import json
import pathlib
import re
import sys
from typing import Iterable


@dataclasses.dataclass(frozen=True)
class WorkPackage:
    id: str
    workstream: str
    unlocks: str
    dependency_status: str
    parallel_safe_with: str
    percent_complete: int
    status: str


@dataclasses.dataclass(frozen=True)
class Recommendation:
    recommended: WorkPackage
    runner_up: WorkPackage | None
    waiting: tuple[WorkPackage, ...]
    first_safe_slice: str
    overlap_risk: str
    reason: str


def strip_markdown(value: str) -> str:
    value = value.replace("<br>", " ")
    value = re.sub(r"`([^`]+)`", r"\1", value)
    value = re.sub(r"\*\*([^*]+)\*\*", r"\1", value)
    return " ".join(value.split())


def parse_percent(value: str) -> int:
    match = re.search(r"(\d+)", value)
    return int(match.group(1)) if match else 0


def split_markdown_row(line: str) -> list[str]:
    return [part.strip() for part in line.strip().strip("|").split("|")]


def parse_work_packages(tracker_text: str) -> list[WorkPackage]:
    packages: list[WorkPackage] = []
    for line in tracker_text.splitlines():
        if not line.startswith("| `"):
            continue

        cells = split_markdown_row(line)
        if len(cells) < 7:
            continue

        package_id = strip_markdown(cells[0])
        if not re.fullmatch(r"(WS|TF)\d+", package_id):
            continue

        packages.append(
            WorkPackage(
                id=package_id,
                workstream=strip_markdown(cells[1]),
                unlocks=strip_markdown(cells[2]),
                dependency_status=strip_markdown(cells[3]),
                parallel_safe_with=strip_markdown(cells[4]),
                percent_complete=parse_percent(cells[5]),
                status=strip_markdown(cells[6]),
            )
        )
    return packages


def parse_recommended_parallel_start(tracker_text: str) -> list[str]:
    start = tracker_text.find("## Recommended Parallel Start")
    if start < 0:
        return []

    section = tracker_text[start:]
    next_heading = section.find("\n## ", 1)
    if next_heading >= 0:
        section = section[:next_heading]

    ids: list[str] = []
    for line in section.splitlines():
        if not line.lstrip().startswith("-"):
            continue
        for match in re.findall(r"`((?:WS|TF)\d+)`", line):
            if match not in ids:
                ids.append(match)
    return ids


def is_waiting(package: WorkPackage) -> bool:
    text = f"{package.dependency_status} {package.status}".lower()
    wait_markers = (
        "blocked",
        "not ready",
        "deferred",
        "do not start",
        "planned after",
        "wait",
        "reject",
    )
    return any(marker in text for marker in wait_markers)


def is_candidate(package: WorkPackage) -> bool:
    if package.percent_complete >= 100:
        return False
    return not is_waiting(package)


def score_package(package: WorkPackage, recommended_order: Iterable[str]) -> int:
    order = list(recommended_order)
    score = 0
    if package.id in order:
        score += 1000 - order.index(package.id) * 25

    text = f"{package.dependency_status} {package.status}".lower()
    if "good to go" in text:
        score += 250
    if "ready" in text:
        score += 175
    if "first slice implemented" in text:
        score += 75
    if "essential" in text:
        score += 60
    if package.id.startswith("TF"):
        score += 20
    score += max(0, 100 - package.percent_complete)
    return score


def first_safe_slice_for(package: WorkPackage) -> str:
    if package.id == "TF9":
        return (
            "Replace the next remaining Patch-shaped editor/layout assumption "
            "with board-profile metadata for existing daisy_patch and "
            "daisy_field behavior only."
        )
    if package.id == "TF14":
        return (
            "Add a thin gate-diagnostics wrapper over existing build_host.cmd "
            "and CTest evidence without changing build semantics."
        )
    if package.id == "TF15":
        return (
            "Extend doctor --json with source/build readiness and known "
            "Windows Path/PATH checks while keeping existing doctor fields."
        )
    if package.id == "WS9":
        return (
            "Plan explicit routing-preset product scope before changing route "
            "semantics."
        )
    if package.id == "WS10":
        return (
            "Continue only for a concrete external-debug consumer that needs "
            "more than the additive CLI debugState payload."
        )
    return f"Start with the smallest test-backed slice of {package.id}: {package.workstream}."


def overlap_risk_for(package: WorkPackage) -> str:
    if package.id == "TF9":
        return "Low if limited to board/editor metadata and existing Patch/Field behavior; avoid routing semantics."
    if package.id == "TF14":
        return "Low if kept as a wrapper over existing build_host.cmd and CTest evidence."
    if package.id == "TF15":
        return "Low if existing doctor fields stay backward compatible."
    if package.id == "WS9":
        return "High with active routing-contract work; start only with explicit routing-preset scope."
    if package.id == "WS10":
        return "Low for additive readback; medium if new commands or state models are added."
    return f"Check ownership against parallel-safe lanes: {package.parallel_safe_with or 'not listed'}."


def recommend_next_work_package(tracker_text: str) -> Recommendation:
    packages = parse_work_packages(tracker_text)
    if not packages:
        raise ValueError("No workstream table rows found")

    by_id = {package.id: package for package in packages}
    recommended_order = parse_recommended_parallel_start(tracker_text)
    candidates = [package for package in packages if is_candidate(package)]
    if not candidates:
        raise ValueError("No ready work packages found")

    candidates.sort(
        key=lambda package: (
            score_package(package, recommended_order),
            package.id in by_id,
        ),
        reverse=True,
    )

    recommended = candidates[0]
    runner_up = candidates[1] if len(candidates) > 1 else None
    waiting = tuple(package for package in packages if is_waiting(package))
    reason = (
        f"{recommended.id} ranks highest from WORKSTREAM_TRACKER.md readiness, "
        f"Recommended Parallel Start ordering, and incomplete status."
    )
    return Recommendation(
        recommended=recommended,
        runner_up=runner_up,
        waiting=waiting,
        first_safe_slice=first_safe_slice_for(recommended),
        overlap_risk=overlap_risk_for(recommended),
        reason=reason,
    )


def recommendation_to_dict(recommendation: Recommendation) -> dict:
    return {
        "recommended": dataclasses.asdict(recommendation.recommended),
        "runnerUp": (
            dataclasses.asdict(recommendation.runner_up)
            if recommendation.runner_up is not None
            else None
        ),
        "waiting": [dataclasses.asdict(package) for package in recommendation.waiting],
        "firstSafeSlice": recommendation.first_safe_slice,
        "overlapRisk": recommendation.overlap_risk,
        "reason": recommendation.reason,
    }


def print_text(recommendation: Recommendation) -> None:
    print(f"Recommended next WP: {recommendation.recommended.id} - {recommendation.recommended.workstream}")
    print(f"Reason: {recommendation.reason}")
    print(f"Dependency status: {recommendation.recommended.dependency_status}")
    print(f"Overlap risk: {recommendation.overlap_risk}")
    print(f"Parallel-safe with: {recommendation.recommended.parallel_safe_with}")
    print(f"First safe slice: {recommendation.first_safe_slice}")
    if recommendation.runner_up is not None:
        print(f"Runner-up: {recommendation.runner_up.id} - {recommendation.runner_up.workstream}")
    if recommendation.waiting:
        wait_ids = ", ".join(package.id for package in recommendation.waiting)
        print(f"Explicitly wait: {wait_ids}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Suggest the next DaisyHost work package from WORKSTREAM_TRACKER.md"
    )
    parser.add_argument(
        "--tracker",
        default="WORKSTREAM_TRACKER.md",
        help="Path to WORKSTREAM_TRACKER.md",
    )
    parser.add_argument("--json", action="store_true", help="Emit JSON")
    args = parser.parse_args(argv)

    tracker_path = pathlib.Path(args.tracker)
    tracker_text = tracker_path.read_text(encoding="utf-8")
    recommendation = recommend_next_work_package(tracker_text)

    if args.json:
        print(json.dumps(recommendation_to_dict(recommendation), indent=2))
    else:
        print_text(recommendation)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
