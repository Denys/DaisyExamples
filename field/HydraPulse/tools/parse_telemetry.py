#!/usr/bin/env python3
"""Parse captured [HPF] text. Reports firmware observations, never measured voltages."""
import argparse
import json
from pathlib import Path
import re
import sys

KINDS = {"boot", "beat", "truth", "key", "knobs", "cv", "midi", "system", "timing", "outputs", "queues"}
TOKEN = re.compile(r"^[a-zA-Z][a-zA-Z0-9_]*=[A-Za-z0-9_.:+/-]+$")

def parse_line(line: str) -> dict | None:
    if not line.startswith("[HPF] "):
        return None
    if len(line) > 2048:
        raise ValueError("oversized telemetry line")
    parts = line.strip().split()
    if len(parts) < 3 or parts[1] not in KINDS:
        raise ValueError("unknown or empty HPF record")
    values = {}
    for token in parts[2:]:
        if not TOKEN.fullmatch(token):
            raise ValueError(f"malformed telemetry token: {token!r}")
        key, value = token.split("=", 1)
        if key in values:
            raise ValueError(f"duplicate telemetry key: {key}")
        values[key] = int(value) if re.fullmatch(r"-?\d+", value) else value
    if parts[1] == "key":
        if (not isinstance(values.get("raw"), int) or not 0 <= values["raw"] < 16
                or values.get("down") not in (0, 1)):
            raise ValueError("invalid key event")
    return {"kind": parts[1], "values": values}

def summarize(lines) -> dict:
    counts = {}
    pressed = set()
    counters = {}
    invalid = 0
    for line in lines:
        try:
            record = parse_line(line)
        except ValueError:
            invalid += 1
            continue
        if record is None:
            continue
        kind, values = record["kind"], record["values"]
        counts[kind] = counts.get(kind, 0) + 1
        if kind == "key" and values["down"] == 1:
            pressed.add(values["raw"])
        for key in ("over", "late", "drops", "mdrops", "sdrops", "faults", "load_pm"):
            if isinstance(values.get(key), int):
                counters[key] = max(counters.get(key, 0), values[key])
    return {"classification": "DERIVED", "basis": "captured firmware telemetry only",
            "records": counts, "invalid_records": invalid, "pressed_raw_indices": sorted(pressed),
            "maximum_reported_counters": counters,
            "not_proven": ["cold boot", "voltage", "audio quality", "absolute CPU timing", "P0 completion"]}

def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("log", type=Path)
    args = p.parse_args()
    try:
        with args.log.open(errors="replace") as f:
            report = summarize(f)
        print(json.dumps(report, indent=2))
        return 1 if report["invalid_records"] or not report["records"] else 0
    except OSError as e:
        print(e, file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
