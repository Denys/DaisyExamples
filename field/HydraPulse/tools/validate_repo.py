#!/usr/bin/env python3
"""Offline structure, provenance-format, preserved R0 and Mermaid-pair checks."""
import argparse
import json
from pathlib import Path
import re
import sys
from common import ROOT, safe_file, sha256
from generate_presets import generate

REQUIRED = (
    "README.md", "CMakeLists.txt", "Makefile", "AGENTS.md", "deps.lock.json",
    "LICENSE_STATUS.md", "third_party/manifest.json", "presets/tutorials.json",
    "src/app/Presets.generated.h", "src/app/Engine.cpp", "src/app/Controller.cpp",
    "src/dsp/Voices.cpp", "firmware/FieldAdapter.h", "firmware/FieldTruth.cpp",
    "firmware/HydraPulse.cpp", "docs/00_EXECUTION_PLAN.md", "docs/01_FOUNDATION_REVIEW.md",
    "docs/USER_MANUAL.md", "docs/CODEX_EXECUTION.md", "docs/QA_PLAN.md",
    "docs/PRESETS.md", "docs/DVPE_INTEGRATION.md", "docs/VALIDATION.md",
    "hardware/control_map.json", "hardware/p0_record.template.json",
    "tests/python/test_tools.py", "tools/build_target.py", "tools/verify_dependencies.py",
    "tools/parse_telemetry.py", "tools/validate_evidence.py", "tools/validate_audio.py",
    "tools/install_additive.py", "references/r0_manifest.json",
)
def validate(base: Path) -> list[str]:
    errors = []
    for name in REQUIRED:
        try:
            safe_file(base, name)
        except ValueError as e:
            errors.append(str(e))
    try:
        lock = json.loads((base / "deps.lock.json").read_text())
        if len(lock["dependencies"]) != 1 or lock["dependencies"][0]["name"] != "libDaisy":
            errors.append("unexpected direct dependencies: review scope and provenance")
        for dep in lock["dependencies"]:
            if not re.fullmatch(r"[0-9a-f]{40}", dep["ref"]):
                errors.append("short or malformed dependency SHA")
        source = json.loads((base / "third_party/manifest.json").read_text())
        if not source.get("sources"):
            errors.append("missing source/provenance records")
        original = json.loads((base / "references/r0_manifest.json").read_text())
        if len(original["files"]) != 29:
            errors.append("R0 preservation inventory must contain all 29 original files")
        for name, digest in original["files"].items():
            if sha256(safe_file(base / "references/r0", name)) != digest:
                errors.append(f"preserved R0 byte mismatch: {name}")
        preset_source = json.loads((base / "presets/tutorials.json").read_text())
        if generate(preset_source) != (base / "src/app/Presets.generated.h").read_text():
            errors.append("generated presets are stale")
    except (OSError, ValueError, KeyError, TypeError) as e:
        errors.append(f"manifest/preset validation failed: {e}")
    diagrams = list((base / "docs/system").glob("*.mmd"))
    if len(diagrams) != 8:
        errors.append("expected eight detailed Mermaid source files")
    for path in diagrams:
        try:
            source = path.read_text().strip()
            md = path.with_suffix(".md").read_text()
            if f"```mermaid\n{source}\n```" not in md:
                errors.append(f"Mermaid source/Markdown mismatch: {path.name}")
            if not source.startswith(("flowchart ", "sequenceDiagram", "stateDiagram-v2")):
                errors.append(f"unexpected diagram type: {path.name}")
        except OSError as e:
            errors.append(str(e))
    for directory in ("src/core", "src/app", "src/dsp"):
        for p in (base / directory).rglob("*"):
            if p.suffix in {".h", ".cpp"} and re.search(
                    r'#include\s*[<"][^>"]*(?:daisy|Arduino|stm32)', p.read_text(), flags=re.I):
                errors.append(f"hardware dependency leaked into host core: {p.relative_to(base)}")
    return errors

def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--root", type=Path, default=ROOT)
    args = p.parse_args()
    errors = validate(args.root)
    for error in errors:
        print(error, file=sys.stderr)
    if not errors:
        print("repository/provenance format, R0 preservation and Mermaid pair consistency: PASS")
        print("upstream checkout verification and Mermaid rendering are separate checks")
    return bool(errors)
if __name__ == "__main__":
    raise SystemExit(main())
