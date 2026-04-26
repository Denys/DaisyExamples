#!/usr/bin/env python3
"""Audit whether a Daisy firmware project is ready for DaisyHost adapter reuse."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


def read_cpp_sources(project: Path) -> dict[Path, str]:
    if project.is_file():
        return {project: project.read_text(encoding="utf-8", errors="replace")}
    sources: dict[Path, str] = {}
    for path in sorted(project.rglob("*")):
        if path.suffix.lower() in {".cpp", ".cc", ".cxx", ".h", ".hpp"}:
            sources[path] = path.read_text(encoding="utf-8", errors="replace")
    return sources


def extract_audio_callback_bodies(text: str) -> list[str]:
    bodies: list[str] = []
    for match in re.finditer(r"(?:void|static\s+void)\s+\w*AudioCallback\s*\(", text):
        start = text.find("{", match.end())
        if start < 0:
            continue
        depth = 0
        for index in range(start, len(text)):
            if text[index] == "{":
                depth += 1
            elif text[index] == "}":
                depth -= 1
                if depth == 0:
                    bodies.append(text[start + 1 : index])
                    break
    return bodies


def classify_project(project: Path) -> dict[str, Any]:
    sources = read_cpp_sources(project)
    combined = "\n".join(sources.values())
    evidence: list[str] = []
    blocking: list[str] = []
    required_actions: list[str] = []

    shared_core_matches = sorted(
        set(re.findall(r'daisyhost/apps/[^"]+\.h', combined))
    )
    if shared_core_matches:
        evidence.extend(shared_core_matches)
    else:
        blocking.append("missing DaisyHost shared app core include")
        required_actions.append(
            "extract portable DSP/control logic behind HostedAppCore before importing"
        )

    if "DaisyField" in combined or "daisy_field.h" in combined:
        evidence.append("uses Daisy Field hardware API")

    if re.search(r"\bint\s+main\s*\(", combined):
        evidence.append("owns firmware main loop")

    callback_bodies: list[str] = []
    for text in sources.values():
        callback_bodies.extend(extract_audio_callback_bodies(text))

    if not callback_bodies:
        blocking.append("missing recognizable AudioCallback")
    else:
        for body in callback_bodies:
            if re.search(r"\bhw\.\s*(Get|Process|display|led_driver|midi|Start)", body):
                blocking.append("audio callback appears to read hardware controls")
                required_actions.append(
                    "move hardware control, MIDI, display, and LED work to the main loop"
                )
                break
        for body in callback_bodies:
            if re.search(r"\b(printf|puts|cout|cerr|fopen|fprintf|StartLog)\b", body):
                blocking.append("audio callback appears to log or print")
                required_actions.append(
                    "remove logging and file I/O from the real-time audio callback"
                )
                break

    if shared_core_matches and not blocking:
        classification = "portable-core-ready"
    elif shared_core_matches:
        classification = "needs-core-extraction"
    elif "DaisyField" in combined or "daisy_field.h" in combined:
        classification = "needs-core-extraction"
    else:
        classification = "not-supported-by-v0"
        required_actions.append("start from a Daisy Field firmware project or shared core")

    return {
        "project": project.stem if project.is_file() else project.name,
        "classification": classification,
        "evidence": evidence,
        "blocking_findings": sorted(set(blocking)),
        "required_actions": sorted(set(required_actions)),
    }


def render_text(report: dict[str, Any]) -> str:
    lines = [
        f"Project: {report['project']}",
        f"Classification: {report['classification']}",
        "",
        "Evidence:",
    ]
    lines.extend(f"- {item}" for item in report["evidence"] or ["none"])
    lines.append("")
    lines.append("Blocking findings:")
    lines.extend(f"- {item}" for item in report["blocking_findings"] or ["none"])
    lines.append("")
    lines.append("Required actions:")
    lines.extend(f"- {item}" for item in report["required_actions"] or ["none"])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Audit a Daisy firmware project for DaisyHost adapter portability."
    )
    parser.add_argument("project", type=Path)
    parser.add_argument("--json", action="store_true", dest="as_json")
    args = parser.parse_args()

    report = classify_project(args.project)
    if args.as_json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(render_text(report))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
