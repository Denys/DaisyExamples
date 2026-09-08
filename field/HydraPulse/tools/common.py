"""Shared local validation helpers. No network or implicit installation."""
from pathlib import Path, PurePosixPath
import hashlib
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()

def safe_file(root: Path, relative: str) -> Path:
    if not isinstance(relative, str):
        raise ValueError("relative file must be a string")
    p = PurePosixPath(relative)
    if not relative or p.is_absolute() or ".." in p.parts or "\\" in relative or ":" in relative:
        raise ValueError(f"unsafe relative file: {relative!r}")
    result = root / relative
    ancestor = result
    while ancestor != root:
        if ancestor.is_symlink():
            raise ValueError(f"symlink in file path: {relative}")
        ancestor = ancestor.parent
    if not result.is_file() or result.is_symlink() or not result.resolve().is_relative_to(root.resolve()):
        raise ValueError(f"missing, symlinked, or escaping file: {relative}")
    return result

def git(repo: Path, *arguments: str) -> str:
    r = subprocess.run(["git", "-C", str(repo), *arguments],
                       text=True, capture_output=True, timeout=60, check=False)
    if r.returncode:
        raise ValueError(f"git {' '.join(arguments)} failed: {r.stderr.strip()}")
    return r.stdout.strip()

def runtime_sources(root: Path = ROOT) -> dict[str, str]:
    result = {"Makefile": sha256(root / "Makefile"),
              "deps.lock.json": sha256(root / "deps.lock.json"),
              "tools/build_target.py": sha256(root / "tools/build_target.py")}
    for directory in ("src", "firmware"):
        for p in sorted((root / directory).rglob("*")):
            if p.is_file() and p.suffix in {".h", ".cpp"}:
                result[p.relative_to(root).as_posix()] = sha256(p)
    return dict(sorted(result.items()))

def manifest_digest(records: dict[str, str]) -> str:
    payload = "".join(f"{v}  {k}\n" for k, v in sorted(records.items())).encode()
    return hashlib.sha256(payload).hexdigest()
