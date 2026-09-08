#!/usr/bin/env python3
"""Validate host scenario metrics and optionally exact repeatability of all rendered files."""
import argparse
import array
import csv
import math
from pathlib import Path
import sys
import wave
from common import sha256

def validate(directory: Path, repeat: Path | None = None) -> list[str]:
    errors = []
    with (directory / "metrics.csv").open(newline="") as f:
        rows = list(csv.DictReader(f))
    if len(rows) != 8 or {r["preset"] for r in rows} != {str(i) for i in range(8)}:
        return ["expected exactly eight distinct tutorial scenarios"]
    for row in rows:
        try:
            values = {k: float(v) for k, v in row.items()}
            if not all(math.isfinite(x) for x in values.values()):
                raise ValueError("nonfinite metric")
            if (values["frames"] != 480000 or values["peak"] >= 0.737 or
                    values["tail_peak"] >= 1e-7 or values["faults"] != 0):
                raise ValueError("amplitude, silence, duration or fault gate failed")
            if row["preset"] == "0" and values["peak"] != 0:
                raise ValueError("Blank Canvas must be silent")
            if row["preset"] != "0" and values["rms"] <= 1e-4:
                raise ValueError("musical scenario unexpectedly silent")
            path = directory / f"preset_{row['preset']}.wav"
            with wave.open(str(path)) as w:
                if (w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()) != (2, 2, 48000, 480000):
                    raise ValueError("wrong PCM format")
                pcm = array.array("h", w.readframes(w.getnframes()))
                if sys.byteorder != "little":
                    pcm.byteswap()
                if len(pcm) != 960000:
                    raise ValueError("truncated PCM payload")
                pcm_peak = max(abs(x) for x in pcm) / 32767.0
                if abs(pcm_peak - values["peak"]) > 0.0001 or pcm_peak >= 0.737:
                    raise ValueError("PCM peak disagrees with the metrics or exceeds bound")
                if any(pcm[-96000:]):
                    raise ValueError("PCM tail must be exactly silent")
            if repeat is not None and sha256(path) != sha256(repeat / path.name):
                raise ValueError("nondeterministic repeated WAV")
        except (ValueError, OSError, wave.Error) as e:
            errors.append(f"preset {row.get('preset')}: {e}")
    if repeat is not None and sha256(directory / "metrics.csv") != sha256(repeat / "metrics.csv"):
        errors.append("metric output differs between repeated runs")
    return errors

def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("renders", type=Path)
    p.add_argument("--repeat", type=Path)
    args = p.parse_args()
    try:
        errors = validate(args.renders, args.repeat)
        for e in errors:
            print(e, file=sys.stderr)
        if not errors:
            print("host sonic scenarios: PASS; listening, aliasing and hardware audio remain separate gates")
        return bool(errors)
    except (KeyError, ValueError, OSError) as e:
        print(e, file=sys.stderr)
        return 1
if __name__ == "__main__":
    raise SystemExit(main())
