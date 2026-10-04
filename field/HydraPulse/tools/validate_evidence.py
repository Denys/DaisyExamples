#!/usr/bin/env python3
"""Check evidence-record integrity. Valid metadata does not establish physical truth."""
import argparse
from datetime import datetime
import json
import math
from pathlib import Path
import re
import sys
from common import safe_file, sha256

CLASSES = {"VERIFIED", "DERIVED", "PROPOSED", "ASSUMED", "HOLD", "NOT_RUN"}

def validate(document: dict, base: Path) -> list[str]:
    errors = []
    if not isinstance(document, dict):
        return ["evidence document must be an object"]
    if document.get("schema_version") != 1:
        errors.append("unsupported evidence schema")
    records = document.get("records")
    if not isinstance(records, list) or not records:
        return errors + ["records must be a nonempty list"]
    seen = set()
    for r in records:
        if not isinstance(r, dict):
            errors.append("record must be an object")
            continue
        label = r.get("id", "<missing>")
        if not isinstance(label, str) or not label or label in seen:
            errors.append("invalid or duplicate id")
        seen.add(str(label))
        status = r.get("classification")
        if not isinstance(status, str) or status not in CLASSES:
            errors.append(f"{label}: invalid classification")
        if not isinstance(r.get("domain"), str) or r["domain"] not in {"hardware", "host", "target-build", "design"}:
            errors.append(f"{label}: invalid domain")
        if not isinstance(r.get("claim"), str) or not r["claim"].strip():
            errors.append(f"{label}: claim missing")
        if status == "VERIFIED" and r.get("domain") == "hardware":
            for field in ("fixture", "operator", "utc", "method", "result"):
                if not isinstance(r.get(field), str) or not r[field].strip():
                    errors.append(f"{label}: hardware {field} missing")
            try:
                timestamp = datetime.fromisoformat(str(r.get("utc", "")).replace("Z", "+00:00"))
                if timestamp.tzinfo is None or timestamp.utcoffset().total_seconds() != 0:
                    raise ValueError("timestamp must be UTC with an explicit zone")
            except ValueError:
                errors.append(f"{label}: invalid UTC timestamp")
            if not re.fullmatch(r"[0-9a-f]{64}", str(r.get("firmware_sha256", ""))):
                errors.append(f"{label}: full firmware SHA-256 missing")
        if isinstance(status, str) and status in {"VERIFIED", "DERIVED"}:
            artifacts = r.get("artifacts")
            if not isinstance(artifacts, list) or not artifacts:
                errors.append(f"{label}: raw artifacts missing")
                continue
            for item in artifacts:
                try:
                    path = safe_file(base, item["path"])
                    if item["sha256"] != sha256(path):
                        raise ValueError("artifact hash mismatch")
                except (ValueError, KeyError, TypeError, OSError) as e:
                    errors.append(f"{label}: {e}")
        # Reject NaN/Infinity in JSON measurements instead of letting a numeric comparison silently pass.
        def finite(obj):
            if isinstance(obj, float):
                return math.isfinite(obj)
            if isinstance(obj, list):
                return all(finite(x) for x in obj)
            if isinstance(obj, dict):
                return all(finite(x) for x in obj.values())
            return True
        if not finite(r):
            errors.append(f"{label}: nonfinite numeric evidence")
    return errors

def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("record", type=Path)
    args = p.parse_args()
    try:
        errors = validate(json.loads(args.record.read_text()), args.record.parent)
        for e in errors:
            print(e, file=sys.stderr)
        if not errors:
            print("evidence structure: PASS (not an independent measurement audit)")
        return bool(errors)
    except (ValueError, OSError) as e:
        print(e, file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
