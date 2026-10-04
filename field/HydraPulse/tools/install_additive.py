#!/usr/bin/env python3
"""Install this overlay into the named DaisyExamples checkout, refusing every differing file."""
import argparse
import json
from pathlib import Path
import re
import sys
from common import ROOT, git, safe_file, sha256

MANIFEST = "field/HydraPulse/PACKAGE_MANIFEST.json"
WORKFLOW = ".github/workflows/hydrapulse.yml"

def inventory(source: Path) -> dict[str, str]:
    doc = json.loads(safe_file(source, MANIFEST).read_text())
    records = doc["files"]
    if not isinstance(records, dict) or not records:
        raise ValueError("empty or malformed package manifest")
    for name, digest in records.items():
        if not (name.startswith("field/HydraPulse/") or name == WORKFLOW):
            raise ValueError(f"out-of-scope payload: {name}")
        path = safe_file(source, name)
        if sha256(path) != digest:
            raise ValueError(f"payload hash mismatch: {name}")
    return {**records, MANIFEST: sha256(source / MANIFEST)}

def compare(source: Path, destination: Path, records: dict) -> dict:
    result = {"new": [], "identical": [], "conflicts": []}
    for name in sorted(records):
        target = destination / name
        # Never traverse a symlink in the destination, including directory symlinks.
        parent = target
        bad_parent = False
        while parent != destination:
            if parent.is_symlink() or (parent != target and parent.exists() and not parent.is_dir()):
                bad_parent = True
                break
            parent = parent.parent
        if bad_parent or (target.exists() and not target.is_file()):
            result["conflicts"].append(name)
        elif target.is_file():
            result["identical" if sha256(target) == records[name] else "conflicts"].append(name)
        else:
            result["new"].append(name)
    return result

def apply(source: Path, destination: Path, result: dict) -> None:
    if result["conflicts"]:
        raise ValueError("conflicting remote/local work requires explicit reconciliation; no files written")
    for name in result["new"]:
        target = destination / name
        target.parent.mkdir(parents=True, exist_ok=True)
        # Exclusive create also rejects a file created after the preflight.
        with target.open("xb") as f:
            f.write(safe_file(source, name).read_bytes())

def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("destination", type=Path, help="root of a local Denys/DaisyExamples Git checkout")
    p.add_argument("--source", type=Path, default=ROOT.parents[1], help="extracted overlay root")
    p.add_argument("--apply", action="store_true", help="add missing files after a clean preflight")
    args = p.parse_args()
    try:
        source, destination = args.source.resolve(), args.destination.resolve()
        if source == destination:
            raise ValueError("source overlay and destination checkout must differ")
        top = Path(git(destination, "rev-parse", "--show-toplevel")).resolve()
        if top != destination:
            raise ValueError("destination must be the Git repository root")
        remote = git(destination, "remote", "get-url", "origin")
        if not re.fullmatch(r"(?:https://github\.com/|git@github\.com:)Denys/DaisyExamples(?:\.git)?/?",
                            remote, flags=re.IGNORECASE):
            raise ValueError(f"unexpected origin: {remote}; expected Denys/DaisyExamples")
        result = compare(source, destination, inventory(source))
        result["origin"] = remote
        try:
            result["head"] = git(destination, "rev-parse", "HEAD")
        except ValueError:
            result["head"] = None
        print(json.dumps(result, indent=2))
        if result["conflicts"]:
            return 1
        if args.apply:
            apply(source, destination, result)
            print(f"added {len(result['new'])} files; no existing file replaced; nothing committed or pushed")
        else:
            print("dry run only; use --apply after reviewing repository instructions and this file comparison")
        return 0
    except (ValueError, KeyError, OSError) as e:
        print(f"overlay install: FAIL: {e}", file=sys.stderr)
        return 1
if __name__ == "__main__":
    raise SystemExit(main())
