from pathlib import Path


ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "Hack_Audio_textbookcode-master"
OUTPUT = ROOT / "Hack_Audio_textbookcode-master_LLM_REFERENCE.md"


def rel(path: Path) -> str:
    return path.relative_to(SOURCE).as_posix()


def line_count(path: Path) -> int:
    return len(path.read_text(encoding="utf-8", errors="replace").splitlines())


def main() -> None:
    chapters = sorted(
        [p for p in SOURCE.iterdir() if p.is_dir() and p.name.startswith("Ch_")],
        key=lambda p: int(p.name.removeprefix("Ch_")),
    )
    code_files = [f for ch in chapters for f in sorted(ch.glob("*.m"))]
    asset_files = [
        f for ch in chapters for f in sorted(ch.iterdir()) if f.is_file() and f.suffix != ".m"
    ]

    lines: list[str] = [
        "# Hack Audio Textbook Code LLM Reference",
        "",
        "Source directory: `Hack_Audio_textbookcode-master/`",
        "",
        "Scope: all MATLAB `.m` source files are embedded verbatim, grouped by chapter. "
        "Non-code assets such as `.wav` files are listed in each chapter manifest and are not inlined.",
        "",
        "Source notice: this folder identifies the material as code from the Hack Audio textbook, "
        "published by Taylor & Francis for educational use. See "
        "`Hack_Audio_textbookcode-master/README.md` for the original notice and links.",
        "",
        "LLM navigation markers:",
        "",
        "- Search for `BEGIN_CHAPTER: Ch_XX` to jump to a chapter.",
        "- Search for `BEGIN_FILE: Ch_XX/name.m` to jump to a specific source file.",
        "- Each source block is fenced as MATLAB and bounded by `BEGIN_FILE` / `END_FILE` comments.",
        "",
        "## Corpus Summary",
        "",
        "| Item | Count |",
        "|---|---:|",
        f"| Chapters | {len(chapters)} |",
        f"| MATLAB source files | {len(code_files)} |",
        f"| Non-code assets | {len(asset_files)} |",
        f"| Embedded source bytes | {sum(f.stat().st_size for f in code_files)} |",
        "",
        "## Chapter Index",
        "",
        "| Chapter | Code files | Assets |",
        "|---|---:|---:|",
    ]

    for chapter in chapters:
        chapter_code = list(chapter.glob("*.m"))
        chapter_assets = [f for f in chapter.iterdir() if f.is_file() and f.suffix != ".m"]
        lines.append(f"| `{chapter.name}` | {len(chapter_code)} | {len(chapter_assets)} |")

    for chapter in chapters:
        chapter_code = sorted(chapter.glob("*.m"))
        chapter_assets = sorted(
            [f for f in chapter.iterdir() if f.is_file() and f.suffix != ".m"]
        )
        lines.extend(
            [
                "",
                f"<!-- BEGIN_CHAPTER: {chapter.name} -->",
                f"## {chapter.name}",
                "",
                "### Manifest",
                "",
                "| Type | Path | Lines | Bytes |",
                "|---|---|---:|---:|",
            ]
        )
        for path in chapter_code:
            lines.append(
                f"| code | `{rel(path)}` | {line_count(path)} | {path.stat().st_size} |"
            )
        for path in chapter_assets:
            lines.append(f"| asset | `{rel(path)}` | - | {path.stat().st_size} |")

        lines.extend(["", "### Source Files"])
        for path in chapter_code:
            marker = rel(path)
            source = path.read_text(encoding="utf-8", errors="replace").replace("\r\n", "\n")
            source = source.replace("\r", "\n")
            lines.extend(
                [
                    "",
                    f"<!-- BEGIN_FILE: {marker} -->",
                    f"#### `{marker}`",
                    "",
                    "````matlab",
                    source.rstrip("\n"),
                    "````",
                    f"<!-- END_FILE: {marker} -->",
                ]
            )
        lines.extend(["", f"<!-- END_CHAPTER: {chapter.name} -->"])

    OUTPUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(OUTPUT)
    print(f"chapters={len(chapters)} code_files={len(code_files)} assets={len(asset_files)}")


if __name__ == "__main__":
    main()
