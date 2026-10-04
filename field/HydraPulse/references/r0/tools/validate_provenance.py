#!/usr/bin/env python3
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SHA40 = re.compile(r"^[0-9a-f]{40}$")


def load(path: str):
    with (ROOT / path).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def validate_entries(entries, container_name: str) -> list[str]:
    errors = []
    for index, entry in enumerate(entries):
        prefix = f"{container_name}[{index}]"
        for field in ("name", "repository", "ref"):
            if not entry.get(field):
                errors.append(f"{prefix}: missing {field}")
        ref = entry.get("ref", "")
        if ref and not SHA40.fullmatch(ref):
            errors.append(f"{prefix}: ref is not a full 40-hex commit SHA: {ref}")
        repo = entry.get("repository", "")
        if repo and not repo.startswith("https://github.com/"):
            errors.append(f"{prefix}: repository is not a GitHub HTTPS URL: {repo}")
    return errors


def main() -> int:
    lock = load("deps.lock.json")
    manifest = load("third_party/manifest.json")

    errors = []
    errors += validate_entries(lock.get("dependencies", []), "dependencies")
    errors += validate_entries(manifest.get("sources", []), "sources")

    direct = {entry["repository"]: entry["ref"] for entry in lock.get("dependencies", [])}
    sources = {entry["repository"]: entry["ref"] for entry in manifest.get("sources", [])}
    for repository, ref in direct.items():
        if repository not in sources:
            errors.append(f"direct dependency {repository} missing from third_party/manifest.json")
        elif sources[repository] != ref:
            errors.append(f"direct dependency {repository} ref mismatch between lock and manifest")

    if errors:
        print("provenance validation: FAIL")
        for error in errors:
            print(f"  - {error}")
        return 1

    print("provenance validation: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
