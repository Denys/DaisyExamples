from __future__ import annotations

import re
import textwrap
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

import fitz


ROOT = Path(__file__).resolve().parent.parent
OUTPUT = ROOT / "docs" / "daisypedal_reference.pdf"


@dataclass
class Section:
    title: str
    body: str


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8").replace("\r\n", "\n")


def clean_markdown(text: str) -> str:
    text = re.sub(r"\[TOC\]\s*", "", text)
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = text.replace("```", "")
    cleaned_lines = []
    for raw_line in text.splitlines():
        line = raw_line.rstrip()
        if line.startswith("#"):
            line = re.sub(r"^#+\s*", "", line).strip()
            if line:
                cleaned_lines.append(line.upper())
                cleaned_lines.append("")
            continue
        cleaned_lines.append(line)
    return "\n".join(cleaned_lines).strip()


def extract_groups(path: Path) -> str:
    text = read_text(path)
    pattern = re.compile(
        r"@defgroup\s+(\S+)\s+([^\n]+).*?@brief\s+([^*]+)",
        re.DOTALL,
    )
    lines = []
    for group_id, title, brief in pattern.findall(text):
        lines.append(f"- {title.strip()} ({group_id.strip()})")
        lines.append(f"  {brief.strip()}")
        lines.append("")
    return "\n".join(lines).strip()


def extract_public_signatures(header_path: Path) -> list[str]:
    lines = read_text(header_path).splitlines()
    inside_public = False
    pending: list[str] = []
    signatures: list[str] = []

    for raw_line in lines:
        stripped = raw_line.strip()
        if stripped == "public:":
            inside_public = True
            pending = []
            continue
        if stripped in {"private:", "protected:"}:
            inside_public = False
            pending = []
            continue
        if not inside_public:
            continue
        if not stripped or stripped.startswith("/**") or stripped.startswith("*") or stripped.startswith("//"):
            continue

        pending.append(stripped)
        joined = " ".join(pending)
        if "(" in joined and (joined.endswith(";") or joined.endswith("{")):
            signature = joined.rstrip("{").rstrip().rstrip(";").strip()
            if signature:
                signatures.append(signature)
            pending = []
        elif joined.endswith("};"):
            pending = []

    return signatures


def extract_header_summary(header_path: Path) -> str:
    text = read_text(header_path)
    file_brief_match = re.search(r"@brief\s+([^\n]+)", text)
    class_match = re.search(r"class\s+(\w+)", text)
    file_brief = file_brief_match.group(1).strip() if file_brief_match else ""
    class_name = class_match.group(1).strip() if class_match else header_path.stem
    signatures = extract_public_signatures(header_path)

    lines = [f"Header: {header_path.as_posix().replace(str(ROOT.as_posix()) + '/', '')}"]
    if file_brief:
        lines.append(f"Summary: {file_brief}")
    lines.append(f"Primary type: {class_name}")
    if signatures:
        lines.append("")
        lines.append("Public API:")
        for signature in signatures[:18]:
            lines.append(f"  - {signature}")
    return "\n".join(lines).strip()


def build_sections() -> list[Section]:
    sections: list[Section] = []

    sections.append(
        Section(
            "Overview",
            clean_markdown(read_text(ROOT / "docs" / "daisypedal_reference_main.md")),
        )
    )
    sections.append(
        Section(
            "Doxygen Groups",
            extract_groups(ROOT / "docs" / "daisypedal_groups.dox"),
        )
    )
    sections.append(
        Section(
            "DaisyPedal Board API",
            extract_header_summary(ROOT / "libDaisy" / "src" / "daisy_pedal.h"),
        )
    )

    module_dir = ROOT / "DaisySP" / "Source" / "DaisyPedal"
    module_bodies = []
    for header in sorted(module_dir.glob("*.h")):
        module_bodies.append(extract_header_summary(header))
    sections.append(Section("DaisySP Modules", "\n\n".join(module_bodies)))

    readme_paths = [
        ROOT / "pedal" / "README.md",
        ROOT / "pedal" / "PassthruBypass" / "README.md",
        ROOT / "pedal" / "NoiseGate" / "README.md",
        ROOT / "pedal" / "PitchDrop" / "README.md",
        ROOT / "pedal" / "PolyOctave" / "README.md",
    ]
    for path in readme_paths:
        sections.append(
            Section(
                f"Example Notes: {path.parent.name if path.parent != ROOT / 'pedal' else 'pedal'}",
                clean_markdown(read_text(path)),
            )
        )

    return sections


class PdfWriter:
    def __init__(self) -> None:
        self.doc = fitz.open()
        self.page = None
        self.width = 595
        self.height = 842
        self.left = 44
        self.right = 44
        self.top = 44
        self.bottom = 44
        self.y = self.top
        self.new_page()

    def new_page(self) -> None:
        self.page = self.doc.new_page(width=self.width, height=self.height)
        self.y = self.top

    def ensure_space(self, lines: int, leading: int) -> None:
        if self.y + (lines * leading) > self.height - self.bottom:
            self.new_page()

    def add_heading(self, text: str, fontsize: int = 16) -> None:
        self.ensure_space(2, fontsize + 4)
        self.page.insert_text((self.left, self.y), text, fontsize=fontsize, fontname="helv")
        self.y += fontsize + 8

    def add_wrapped(self, text: str, width: int = 92, fontsize: int = 10, leading: int = 13) -> None:
        paragraphs = text.split("\n")
        for paragraph in paragraphs:
            if not paragraph.strip():
                self.y += leading // 2
                continue

            if paragraph.startswith("  - "):
                wrapped = textwrap.wrap(
                    paragraph[4:],
                    width=max(20, width - 4),
                    subsequent_indent="    ",
                    initial_indent="  - ",
                )
            elif paragraph.startswith("- "):
                wrapped = textwrap.wrap(
                    paragraph[2:],
                    width=max(20, width - 2),
                    subsequent_indent="  ",
                    initial_indent="- ",
                )
            else:
                wrapped = textwrap.wrap(paragraph, width=width) or [""]

            self.ensure_space(len(wrapped), leading)
            for line in wrapped:
                self.page.insert_text((self.left, self.y), line, fontsize=fontsize, fontname="cour")
                self.y += leading

    def save(self, path: Path) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        self.doc.save(path)
        self.doc.close()


def main() -> None:
    writer = PdfWriter()

    writer.add_heading("DaisyPedal Reference", fontsize=22)
    writer.add_wrapped(
        "Fallback PDF build generated without Doxygen/LaTeX. Content is assembled "
        "from the current Pedal markdown docs and public headers.",
        width=84,
        fontsize=11,
        leading=14,
    )
    writer.add_wrapped(
        f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
        width=84,
        fontsize=10,
        leading=13,
    )
    writer.y += 8

    for section in build_sections():
        if writer.y > writer.height - 160:
            writer.new_page()
        writer.add_heading(section.title, fontsize=16)
        writer.add_wrapped(section.body)
        writer.y += 8

    writer.save(OUTPUT)
    print(f"Wrote {OUTPUT}")


if __name__ == "__main__":
    main()
