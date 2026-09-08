#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

REQUIRED = [
    "README.md",
    "CMakeLists.txt",
    "deps.lock.json",
    "third_party/manifest.json",
    "src/core/Pattern16.h",
    "src/core/StepSequencer.h",
    "src/core/SampleAccurateStepClock.h",
    "src/core/ControlPickup.h",
    "tests/native/CMakeLists.txt",
    "docs/ARCHITECTURE.md",
    "docs/POC_SEQUENCE.md",
    "docs/TEST_STRATEGY.md",
    "docs/SOURCE_REVIEW.md",
    "docs/STATUS.md",
    "docs/LOCAL_VALIDATION.md",
    "firmware/field_truth/README.md",
    "evidence/README.md",
]

FORBIDDEN_PUBLIC_EXTENSIONS = {".pdf"}


def main() -> int:
    missing = [path for path in REQUIRED if not (ROOT / path).is_file()]
    if missing:
        print("missing required files:")
        for path in missing:
            print(f"  - {path}")
        return 1

    forbidden = []
    for path in ROOT.rglob("*"):
        if path.is_file() and path.suffix.lower() in FORBIDDEN_PUBLIC_EXTENSIONS:
            forbidden.append(path.relative_to(ROOT).as_posix())

    if forbidden:
        print("unexpected binary/manual-like files in bootstrap:")
        for path in forbidden:
            print(f"  - {path}")
        return 1

    print("repository structure: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
