#!/usr/bin/env python3
"""
List repo project roots by relevant file activity.

This is intended to help refresh LATEST_PROJECTS.md without overfitting to one
subtree such as MyProjects/_projects.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parent.parent

PINNED_WORKSPACES = [
    "DaisyDAFX",
    "pedal",
    "DAISY_QAE",
    "MyProjects/DAFX_2_Daisy_lib",
    "MyProjects/_projects",
]

COLLECTION_ROOTS = [
    "pedal",
    "MyProjects/_projects",
    "MyProjects/_experiments",
]

IGNORED_DIRS = {
    ".git",
    ".github",
    ".idea",
    ".tmp",
    ".vscode",
    ".worktrees",
    "__pycache__",
    "build",
    "dist",
    "node_modules",
}

RELEVANT_SUFFIXES = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".docx",
    ".h",
    ".hpp",
    ".ino",
    ".md",
    ".pdf",
    ".py",
    ".sh",
    ".txt",
    ".toml",
    ".xlsx",
    ".yaml",
    ".yml",
}

RELEVANT_FILENAMES = {
    "Makefile",
    "CMakeLists.txt",
    "CHECKPOINT.md",
    "CONTROLS.md",
    "README.md",
}


@dataclass
class Candidate:
    path: Path
    kind: str


@dataclass
class Activity:
    candidate: Candidate
    latest_file: Path
    latest_mtime: float


def is_relevant_file(path: Path) -> bool:
    return path.name in RELEVANT_FILENAMES or path.suffix.lower() in RELEVANT_SUFFIXES


def iter_candidate_files(root: Path) -> Iterable[Path]:
    for path in root.rglob("*"):
        if path.is_dir():
            continue
        relative_parts = path.relative_to(root).parts
        if any(part in IGNORED_DIRS for part in relative_parts[:-1]):
            continue
        if is_relevant_file(path):
            yield path


def collect_candidates() -> list[Candidate]:
    seen: set[Path] = set()
    candidates: list[Candidate] = []

    for rel in PINNED_WORKSPACES:
        path = REPO_ROOT / rel
        if path.exists():
            resolved = path.resolve()
            if resolved not in seen:
                seen.add(resolved)
                candidates.append(Candidate(path=path, kind="pinned"))

    for rel in COLLECTION_ROOTS:
        root = REPO_ROOT / rel
        if not root.exists():
            continue
        for child in sorted(root.iterdir()):
            if not child.is_dir() or child.name in IGNORED_DIRS:
                continue
            resolved = child.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            candidates.append(Candidate(path=child, kind="leaf"))

    return candidates


def summarize_activity(candidate: Candidate) -> Activity | None:
    latest_file: Path | None = None
    latest_mtime = -1.0

    for file_path in iter_candidate_files(candidate.path):
        mtime = file_path.stat().st_mtime
        if mtime > latest_mtime:
            latest_mtime = mtime
            latest_file = file_path

    if latest_file is None:
        return None

    return Activity(candidate=candidate, latest_file=latest_file, latest_mtime=latest_mtime)


def format_timestamp(timestamp: float) -> str:
    return datetime.fromtimestamp(timestamp).strftime("%Y-%m-%d %H:%M:%S")


def main() -> int:
    parser = argparse.ArgumentParser(description="List recent repo project roots.")
    parser.add_argument(
        "--limit",
        type=int,
        default=20,
        help="Maximum number of non-pinned rows to print. Pinned rows are always included.",
    )
    args = parser.parse_args()

    activities = [
        activity
        for candidate in collect_candidates()
        if (activity := summarize_activity(candidate)) is not None
    ]
    pinned = [item for item in activities if item.candidate.kind == "pinned"]
    leaf = [item for item in activities if item.candidate.kind != "pinned"]
    pinned.sort(key=lambda item: item.latest_mtime, reverse=True)
    leaf.sort(key=lambda item: item.latest_mtime, reverse=True)
    ordered = pinned + leaf[: args.limit]

    print("kind\tlatest_timestamp\tproject_root\tlatest_relevant_file")
    for activity in ordered:
        project_root = activity.candidate.path.relative_to(REPO_ROOT).as_posix()
        latest_file = activity.latest_file.relative_to(REPO_ROOT).as_posix()
        print(
            f"{activity.candidate.kind}\t"
            f"{format_timestamp(activity.latest_mtime)}\t"
            f"{project_root}\t"
            f"{latest_file}"
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
