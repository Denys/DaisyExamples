from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.units import mm
from reportlab.pdfgen import canvas
from pypdf import PdfReader


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "MyProjects" / "_projects" / "field_mixer" / "Hydra_Field_Mixer_A4_Controls.pdf"

PAGE_W, PAGE_H = landscape(A4)
MARGIN = 9 * mm
GAP = 4 * mm


PALETTE = {
    "ink": colors.HexColor("#111827"),
    "muted": colors.HexColor("#4b5563"),
    "line": colors.HexColor("#d1d5db"),
    "panel": colors.HexColor("#f8fafc"),
    "hydra": colors.HexColor("#2563eb"),
    "edge": colors.HexColor("#dc2626"),
    "delay": colors.HexColor("#7c3aed"),
    "safe": colors.HexColor("#059669"),
    "warn": colors.HexColor("#b45309"),
}


def draw_text(c, x, y, text, size=8, font="Helvetica", color=None):
    c.setFont(font, size)
    c.setFillColor(color or PALETTE["ink"])
    c.drawString(x, y, text)


def draw_right(c, x, y, text, size=8, font="Helvetica", color=None):
    c.setFont(font, size)
    c.setFillColor(color or PALETTE["ink"])
    c.drawRightString(x, y, text)


def draw_center(c, x, y, text, size=8, font="Helvetica", color=None):
    c.setFont(font, size)
    c.setFillColor(color or PALETTE["ink"])
    c.drawCentredString(x, y, text)


def panel(c, x, y, w, h, title, accent):
    c.setFillColor(PALETTE["panel"])
    c.setStrokeColor(PALETTE["line"])
    c.roundRect(x, y - h, w, h, 4, fill=1, stroke=1)
    c.setFillColor(accent)
    c.roundRect(x, y - 8.5 * mm, w, 8.5 * mm, 4, fill=1, stroke=0)
    c.setFillColor(accent)
    c.rect(x, y - 8.5 * mm, w, 3.5 * mm, fill=1, stroke=0)
    draw_text(c, x + 3 * mm, y - 5.8 * mm, title, 9, "Helvetica-Bold", colors.white)


def table(c, x, y, w, rows, col1=18 * mm, row_h=5.1 * mm, font_size=6.9):
    c.setStrokeColor(PALETTE["line"])
    c.setLineWidth(0.35)
    yy = y
    for idx, (left, right) in enumerate(rows):
        if idx % 2:
            c.setFillColor(colors.HexColor("#ffffff"))
        else:
            c.setFillColor(colors.HexColor("#f3f4f6"))
        c.rect(x, yy - row_h + 1, w, row_h, fill=1, stroke=0)
        draw_text(c, x + 1.4 * mm, yy - row_h + 2.2 * mm, left, font_size, "Helvetica-Bold")
        draw_text(c, x + col1, yy - row_h + 2.2 * mm, right, font_size)
        c.line(x, yy - row_h + 1, x + w, yy - row_h + 1)
        yy -= row_h
    return yy


def bullet(c, x, y, text, size=6.8, color=None):
    c.setFillColor(color or PALETTE["ink"])
    c.circle(x + 1.1 * mm, y + 1.1, 0.75, fill=1, stroke=0)
    draw_text(c, x + 3.2 * mm, y, text, size, color=color)


def key_table(c, x, y, w, title, rows, accent):
    draw_text(c, x, y, title, 7.3, "Helvetica-Bold", accent)
    return table(c, x, y - 2.0 * mm, w, rows, col1=10 * mm, row_h=4.65 * mm, font_size=6.45)


def draw_signal_flow(c, x, y, w):
    labels = [
        ("Hydra IN L", PALETTE["hydra"]),
        ("Trim/HPF/Tone/Gate", PALETTE["safe"]),
        ("Mix + Delay", PALETTE["delay"]),
        ("Limiter", PALETTE["safe"]),
        ("OUT L/R + HP", PALETTE["ink"]),
    ]
    box_w = (w - 4 * mm) / len(labels)
    for idx, (label, color) in enumerate(labels):
        bx = x + idx * box_w
        c.setFillColor(colors.white)
        c.setStrokeColor(color)
        c.roundRect(bx, y - 8 * mm, box_w - 1.2 * mm, 8 * mm, 3, fill=1, stroke=1)
        draw_center(c, bx + (box_w - 1.2 * mm) / 2, y - 5.1 * mm, label, 5.9, "Helvetica-Bold", color)
        if idx < len(labels) - 1:
            c.setStrokeColor(PALETTE["muted"])
            c.line(bx + box_w - 2 * mm, y - 4 * mm, bx + box_w - 0.7 * mm, y - 4 * mm)
    draw_text(c, x, y - 12.2 * mm, "Edge IN R uses the same cleanup path, then ducks under Hydra when B3 is on.", 6.5, color=PALETTE["muted"])


def build():
    c = canvas.Canvas(str(OUT), pagesize=landscape(A4))
    c.setTitle("Hydra Field Mixer A4 Controls")
    c.setAuthor("Codex")

    # Header
    draw_text(c, MARGIN, PAGE_H - 10 * mm, "Hydra Field Mixer", 18, "Helvetica-Bold")
    draw_text(c, MARGIN, PAGE_H - 15.5 * mm, "Daisy Field performance mixer for Hydra Explorer + Edge", 8.5, color=PALETTE["muted"])
    draw_right(c, PAGE_W - MARGIN, PAGE_H - 10 * mm, "A4 landscape quick reference", 8.5, "Helvetica-Bold", PALETTE["muted"])
    draw_right(c, PAGE_W - MARGIN, PAGE_H - 15.5 * mm, "Common mix to OUT L, OUT R, and headphones", 7.5, color=PALETTE["safe"])

    col_w = (PAGE_W - 2 * MARGIN - 2 * GAP) / 3
    x1 = MARGIN
    x2 = x1 + col_w + GAP
    x3 = x2 + col_w + GAP
    top = PAGE_H - 20 * mm

    # Column 1
    panel(c, x1, top, col_w, 171 * mm, "Connections + Knobs", PALETTE["hydra"])
    y = top - 12 * mm
    bullet(c, x1 + 3 * mm, y, "Hydra Explorer -> Field IN L. Edge -> Field IN R.", 7.0)
    y -= 5.2 * mm
    bullet(c, x1 + 3 * mm, y, "OUT L, OUT R, and headphones carry the same protected common mix.", 7.0)
    y -= 5.2 * mm
    bullet(c, x1 + 3 * mm, y, "Start with K8 low, raise K1/K2, then set K8 master.", 7.0)
    y -= 7.5 * mm

    draw_text(c, x1 + 3 * mm, y, "Main bank - no switch held", 7.8, "Helvetica-Bold", PALETTE["hydra"])
    y = table(c, x1 + 3 * mm, y - 2 * mm, col_w - 6 * mm, [
        ("K1", "Hydra level"),
        ("K2", "Edge level"),
        ("K3", "Hydra tone: warm - neutral - bright"),
        ("K4", "Edge tone: warm - neutral - bright"),
        ("K5", "Hydra delay send"),
        ("K6", "Edge delay send"),
        ("K7", "Delay return"),
        ("K8", "Master level"),
    ], col1=11 * mm, row_h=5.45 * mm)
    y -= 4.5 * mm
    draw_text(c, x1 + 3 * mm, y, "Alt bank - hold SW1", 7.8, "Helvetica-Bold", PALETTE["safe"])
    y = table(c, x1 + 3 * mm, y - 2 * mm, col_w - 6 * mm, [
        ("K1", "Hydra input trim"),
        ("K2", "Edge input trim"),
        ("K3", "Hydra high-pass"),
        ("K4", "Edge high-pass"),
        ("K5", "Delay time, approx. 60-850 ms"),
        ("K6", "Delay feedback, clamped safe"),
        ("K7", "Delay tone"),
        ("K8", "Ducking amount"),
    ], col1=11 * mm, row_h=5.45 * mm)
    y -= 4.5 * mm
    draw_text(c, x1 + 3 * mm, y, "Utility layer - hold SW2", 7.8, "Helvetica-Bold", PALETTE["warn"])
    table(c, x1 + 3 * mm, y - 2 * mm, col_w - 6 * mm, [
        ("K1", "Hydra mute fade time"),
        ("K2", "Edge mute fade time"),
        ("K3", "Delay throw amount"),
        ("K4", "Delay freeze feedback"),
        ("K5", "Limiter threshold"),
        ("K6", "Output saturation amount"),
        ("K7", "Input gate threshold"),
        ("K8", "Output safety ceiling"),
    ], col1=11 * mm, row_h=5.45 * mm)

    # Column 2
    panel(c, x2, top, col_w, 171 * mm, "Keys + Performance", PALETTE["delay"])
    y = top - 12 * mm
    y = key_table(c, x2 + 3 * mm, y, col_w - 6 * mm, "A row", [
        ("A1", "Hydra mute"),
        ("A2", "Edge mute"),
        ("A3", "Delay mute"),
        ("A4", "Master mute"),
        ("A5", "Hydra solo"),
        ("A6", "Edge solo"),
        ("A7", "Bypass modifiers, keep mixer live"),
        ("A8", "Panic: clear mutes, solos, freeze, throw"),
    ], PALETTE["hydra"])
    y -= 5 * mm
    y = key_table(c, x2 + 3 * mm, y, col_w - 6 * mm, "B row", [
        ("B1", "Delay freeze"),
        ("B2", "Delay throw"),
        ("B3", "Edge ducking on/off"),
        ("B4", "Soft saturation on/off"),
        ("B5", "Scene 1: clean mix"),
        ("B6", "Scene 2: Hydra lead"),
        ("B7", "Scene 3: Edge lead"),
        ("B8", "Scene 4: delay performance"),
    ], PALETTE["delay"])
    y -= 5 * mm
    draw_text(c, x2 + 3 * mm, y, "Scenes", 7.8, "Helvetica-Bold", PALETTE["delay"])
    y = table(c, x2 + 3 * mm, y - 2 * mm, col_w - 6 * mm, [
        ("B5", "Balanced, dry-safe startup sound"),
        ("B6", "Hydra louder, Edge support, modest delay"),
        ("B7", "Edge louder, Hydra support, Edge delay"),
        ("B8", "Higher sends, return, and feedback"),
    ], col1=11 * mm, row_h=5.6 * mm, font_size=6.6)
    y -= 5 * mm
    draw_text(c, x2 + 3 * mm, y, "OLED + LEDs", 7.8, "Helvetica-Bold", PALETTE["safe"])
    y -= 5 * mm
    bullet(c, x2 + 3 * mm, y, "Overview shows active bank, source status, and meters.", 6.8)
    y -= 5 * mm
    bullet(c, x2 + 3 * mm, y, "Edit zoom appears after parameter changes.", 6.8)
    y -= 5 * mm
    bullet(c, x2 + 3 * mm, y, "Knob LEDs show stored values; dim means not captured yet.", 6.8)
    y -= 5 * mm
    bullet(c, x2 + 3 * mm, y, "Mute keys stay on; solo and panic keys blink.", 6.8)

    # Column 3
    panel(c, x3, top, col_w, 171 * mm, "Startup + Troubleshooting", PALETTE["safe"])
    y = top - 12 * mm
    draw_text(c, x3 + 3 * mm, y, "Startup defaults", 7.8, "Helvetica-Bold", PALETTE["safe"])
    y = table(c, x3 + 3 * mm, y - 2 * mm, col_w - 6 * mm, [
        ("Hydra", "Level 75%, send 10%, tone neutral"),
        ("Edge", "Level 70%, send 15%, tone slightly warm"),
        ("Delay", "Return 20%, feedback 25%"),
        ("Cleanup", "Hydra HPF 133 Hz, Edge HPF 158 Hz"),
        ("Safety", "Input gate 45%, limiter on, master 80%"),
        ("Mods", "Ducking off, saturation off"),
    ], col1=17 * mm, row_h=5.6 * mm, font_size=6.55)
    y -= 5 * mm
    draw_text(c, x3 + 3 * mm, y, "No-input tone / hum", 7.8, "Helvetica-Bold", PALETTE["warn"])
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "Unplugged Field line inputs can float and sound like hum.", 6.8, PALETTE["warn"])
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "Raise SW2+K7 for stronger input gate.", 6.8)
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "Raise SW1+K3/K4 for more Hydra/Edge high-pass.", 6.8)
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "Turn K1/K2 down when sources are disconnected.", 6.8)
    y -= 7 * mm
    draw_text(c, x3 + 3 * mm, y, "Signal flow", 7.8, "Helvetica-Bold", PALETTE["ink"])
    draw_signal_flow(c, x3 + 3 * mm, y - 3 * mm, col_w - 6 * mm)
    y -= 25 * mm
    draw_text(c, x3 + 3 * mm, y, "Live-use order", 7.8, "Helvetica-Bold", PALETTE["ink"])
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "1. Set K8 low; connect Hydra and Edge.", 6.8)
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "2. Bring up K1/K2, then K8 master.", 6.8)
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "3. Shape with K3/K4; send with K5/K6.", 6.8)
    y -= 5 * mm
    bullet(c, x3 + 3 * mm, y, "4. Use B1/B2 for delay gestures, A8 for panic.", 6.8)

    # Footer
    c.setStrokeColor(PALETTE["line"])
    c.line(MARGIN, 7.5 * mm, PAGE_W - MARGIN, 7.5 * mm)
    draw_text(c, MARGIN, 4.2 * mm, "Hydra Field Mixer / field_mixer - one-page controls", 6.2, color=PALETTE["muted"])
    draw_right(c, PAGE_W - MARGIN, 4.2 * mm, "Print: A4 landscape, fit to page", 6.2, color=PALETTE["muted"])

    c.showPage()
    c.save()

    reader = PdfReader(str(OUT))
    page = reader.pages[0]
    box = page.mediabox
    print(f"wrote: {OUT}")
    print(f"pages: {len(reader.pages)}")
    print(f"page_size_points: {float(box.width):.2f} x {float(box.height):.2f}")


if __name__ == "__main__":
    build()
