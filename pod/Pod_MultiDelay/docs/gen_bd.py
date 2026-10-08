"""Generate the Pod_MultiDelay v0.3 block diagram (draw.io, two pages)."""
import sys
from xml.sax.saxutils import escape

OUT = sys.argv[1]

# styles
BLK = "rounded=1;whiteSpace=wrap;html=1;fontSize=12;"
AUD = BLK + "fillColor=#dae8fc;strokeColor=#6c8ebf;"
CTL = BLK + "fillColor=#d5e8d4;strokeColor=#82b366;"
MEM = "shape=cylinder3;whiteSpace=wrap;html=1;boundedLbl=1;size=10;fontSize=11;fillColor=#fff2cc;strokeColor=#d6b656;"
HW = BLK + "fillColor=#f5f5f5;strokeColor=#666666;fontStyle=1;"
REF = BLK + "fillColor=#e1d5e7;strokeColor=#9673a6;"
SUM = "ellipse;whiteSpace=wrap;html=1;aspect=fixed;fontSize=16;fontStyle=1;fillColor=#ffffff;strokeColor=#000000;"
GAIN = "triangle;whiteSpace=wrap;html=1;fontSize=11;fillColor=#ffffff;strokeColor=#000000;"
SW = BLK + "fillColor=#ffe6cc;strokeColor=#d79b00;"
NOTE = "text;html=1;align=left;verticalAlign=top;whiteSpace=wrap;fontSize=11;"
TITLE = "text;html=1;align=left;verticalAlign=middle;fontSize=18;fontStyle=1;"
GRP = "rounded=1;whiteSpace=wrap;html=1;dashed=1;fillColor=none;verticalAlign=top;align=left;spacingLeft=8;fontStyle=1;fontSize=13;"
EDGE = "edgeStyle=orthogonalEdgeStyle;rounded=0;html=1;endArrow=block;endFill=1;fontSize=10;"
DASH = EDGE + "dashed=1;"


class Page:
    def __init__(self, name):
        self.name, self.cells, self.n = name, [], 0

    def _id(self):
        self.n += 1
        return f"{self.name[:3]}{self.n}"

    def box(self, label, x, y, w, h, style):
        i = self._id()
        self.cells.append(f'<mxCell id="{i}" value="{escape(label)}" style="{style}" vertex="1" parent="1">'
                          f'<mxGeometry x="{x}" y="{y}" width="{w}" height="{h}" as="geometry"/></mxCell>')
        return i

    def edge(self, a, b, label="", style=EDGE, pts=()):
        i = self._id()
        p = "".join(f'<mxPoint x="{x}" y="{y}"/>' for x, y in pts)
        arr = f'<Array as="points">{p}</Array>' if pts else ""
        self.cells.append(f'<mxCell id="{i}" value="{escape(label)}" style="{style}" edge="1" parent="1" source="{a}" target="{b}">'
                          f'<mxGeometry relative="1" as="geometry">{arr}</mxGeometry></mxCell>')
        return i

    def xml(self):
        body = "".join(self.cells)
        return (f'<diagram name="{escape(self.name)}" id="{self.name[:3]}"><mxGraphModel grid="1" gridSize="10" '
                f'page="1" pageWidth="1654" pageHeight="1169"><root><mxCell id="0"/><mxCell id="1" parent="0"/>'
                f'{body}</root></mxGraphModel></diagram>')


# ---------------------------------------------------------------- page 1: system
s = Page("System")
s.box("Pod_MultiDelay v0.3 — system block diagram (source: DaisyExamples branch claude/pod-multidelay-v1 @ 7b2fbfd1)",
      20, 10, 1300, 30, TITLE)

# hardware row
pots = s.box("K1, K2<br>(ADC, 1 kHz scan)", 40, 90, 140, 50, HW)
enc = s.box("Encoder<br>turn + click", 40, 160, 140, 50, HW)
btn = s.box("B1, B2 (B2 = SHIFT)", 40, 230, 140, 50, HW)
leds = s.box("LED1 mode color<br>LED2 page / status", 40, 320, 140, 50, HW)

# main loop group
s.box("Main loop — 1 kHz, may be preempted by audio", 220, 60, 660, 370, GRP)
ev = s.box("Events<br>pots, turn, click,<br>B1/B2 edges, time", 240, 100, 130, 70, CTL)
ctl = s.box("<b>Controls::Step</b> (Controls.h)<br>• until-moved gate 0.012, rearm on page/mode/SHIFT<br>"
            "• per-mode bank of 5 slots<br>• DIGI: SHIFT+K1 ratio, SHIFT+K2 E2 fb, SHIFT+B1 relink<br>"
            "• SHIFT+turn: DIGI config / FREEZE op<br>• tap tempo (B2 short), bypass (B1), trails<br>• TIME deadband 0.004 (DIGI)",
            400, 90, 330, 140, CTL + "align=left;spacingLeft=8;")
snap = s.box("<b>Snapshot</b><br>mode, epoch, slots[5], config,<br>ratio, feedbackE2, command,<br>bypass, trails",
             400, 260, 200, 80, CTL)
pub = s.box("Publish<br>IRQ-masked copy", 660, 255, 130, 50, CTL)
svc = s.box("<b>Demo::Service</b> (only while audio is Paused)<br>mode change → engine.SetMode / Reset,<br>DigiMono.Reset; FREEZE commands",
            620, 335, 250, 75, CTL)
ledf = s.box("Leds()", 240, 330, 100, 40, CTL)

s.edge(pots, ev)
s.edge(enc, ev)
s.edge(btn, ev)
s.edge(ev, ctl)
s.edge(ctl, snap)
s.edge(snap, pub)
s.edge(snap, svc, "", EDGE + "exitX=1;exitY=0.85;entryX=0;entryY=0.5;", pts=((610, 328), (610, 372)))
s.edge(ledf, leds)

# audio group
s.box("Audio callback — 48 kHz, 48-frame blocks (Demo::Process)", 220, 450, 1420, 350, GRP)
inp = s.box("IN L<br>(IN R ignored)", 40, 565, 140, 50, HW)
shared = s.box("published Snapshot<br>(read once per block)", 240, 475, 160, 50, AUD)
san = s.box("finite + clamp ±1", 240, 565, 130, 50, AUD)
byp = s.box("bypass smoother<br>active 0↔1", 400, 565, 130, 50, AUD)
send = s.box("×", 560, 570, 40, 40, SUM)
sel = s.box("mode ?", 640, 565, 80, 50, SW)
mapb = s.box("<b>DigiMap()</b><br>slots → D1, D2, f1, f2, LP Hz", 790, 475, 220, 50, AUD)
digi = s.box("<b>DigiMono</b> (DigiMono.h)<br>DIGI contract v1<br>SINGLE / SERIES / PARALLEL<br>(detail: page 2)",
             790, 550, 220, 80, AUD)
ref = s.box("<b>daisyhost::PedalDelayEngine</b><br>TAPE / MOD / REV / FREEZE<br>(DaisyHost, unchanged from v0.1)",
            790, 670, 220, 60, REF)
mix = s.box("<b>mixer</b><br>DIGI: y = x + MIX·wet<br>others: x·(1−a·MIX) + MIX·wet<br>trails off: wet × active",
            1080, 545, 210, 90, AUD)
fade = s.box("<b>fade handshake</b><br>240 samples<br>Running / FadingOut /<br>Paused (dry) / FadingIn",
             1320, 545, 160, 90, AUD)
clip = s.box("clamp ±1", 1510, 565, 100, 50, AUD)
outp = s.box("OUT L = OUT R<br>(mono)", 1500, 680, 120, 50, HW)
s.box("SDRAM history<br>2 × 120 008 float<br>(DigiMono E1/E2 or engine)", 1080, 465, 180, 70, MEM)
s.box("SDRAM freezeLoop<br>2 × 96 008 float (engine)", 1290, 465, 180, 70, MEM)

s.edge(inp, san)
s.edge(san, byp)
s.edge(byp, send)
s.edge(send, sel, "send")
s.edge(sel, digi, "DIGI", EDGE + "entryX=0;entryY=0.5;")
s.edge(sel, ref, "TAPE…FREEZE", EDGE + "exitX=0.5;exitY=1;entryX=0;entryY=0.5;", pts=((680, 700),))
s.edge(mapb, digi)
s.edge(digi, mix, "wet", EDGE + "exitX=1;exitY=0.5;entryX=0;entryY=0.5;")
s.edge(ref, mix, "wet (L)", EDGE + "exitX=1;exitY=0.5;entryX=0;entryY=0.85;", pts=((1045, 700), (1045, 621)))
s.edge(san, mix, "dry x", DASH + "exitX=0.5;exitY=1;entryX=0.5;entryY=1;", pts=((305, 770), (1185, 770)))
s.edge(mix, fade)
s.edge(fade, clip)
s.edge(clip, outp)
s.edge(pub, shared, "", DASH + "exitX=0.5;exitY=1;entryX=0.5;entryY=0;", pts=((725, 320), (900, 320), (900, 440), (320, 440)))
s.edge(shared, mapb, "slots, ratio, feedbackE2, config", DASH)
s.edge(svc, ref, "Reset / SetMode", DASH + "exitX=0.5;exitY=1;entryX=0.5;entryY=1;", pts=((745, 750), (900, 750)))
s.box("Sample rate 48 kHz, block 48. BOOT_NONE, FLASH 103 868 B (79.24 %). "
      "A mode change pauses audio (fade to dry), lets main reset the engine, then fades back: tails are discarded. "
      "A DIGI config change needs no pause and keeps tails (contract v1 §7). "
      "Evidence: custom-pedals delay/runs/2026-10-07_pod_multidelay_v0_3_digi_contract.md.",
      220, 815, 1200, 50, NOTE)

# ---------------------------------------------------------------- page 2: DIGI engine
d = Page("DIGI engine")
d.box("DigiMono — mono-first DIGI contract v1 (per-engine law = DAFX DigitalDelayNode @ 73976da, mono)",
      20, 10, 1300, 30, TITLE)

x = d.box("x<br>(send)", 30, 260, 70, 50, HW)

# E1
d.box("Engine E1", 150, 60, 700, 190, GRP)
s1 = d.box("+", 190, 120, 40, 40, SUM)
h1 = d.box("history 1<br>z<sup>−D1</sup><br>(integer, read before write)", 270, 110, 170, 60, MEM.replace("cylinder3", "rect"))
hp1 = d.box("HP 40 Hz<br>(one pole)", 480, 180, 110, 45, AUD)
lp1 = d.box("LP COLOR<br>off … 2 kHz", 620, 180, 110, 45, AUD)
f1 = d.box("f1", 760, 182, 50, 40, GAIN + "direction=west;")
t1 = d.box("tap1", 760, 120, 60, 40, AUD)
d.edge(x, s1, "in1 = x", pts=((125, 285), (125, 140)))
d.edge(s1, h1)
d.edge(h1, t1)
d.edge(t1, hp1, "", EDGE, pts=((790, 170), (535, 170)))
d.edge(hp1, lp1)
d.edge(lp1, f1)
d.edge(f1, s1, "", EDGE, pts=((785, 240), (210, 240)))

# router
r = d.box("<b>in2 router</b><br>SINGLE: 0<br>SERIES: tap1<br>PARALLEL: x", 190, 300, 150, 90, SW)
d.edge(x, r, pts=((115, 345),))
d.edge(t1, r, "", DASH, pts=((880, 140), (880, 280), (265, 280)))

# E2
d.box("Engine E2", 150, 410, 700, 190, GRP)
s2 = d.box("+", 190, 470, 40, 40, SUM)
h2 = d.box("history 2<br>z<sup>−D2</sup>, D2 = round(r·D1)", 270, 460, 170, 60, MEM.replace("cylinder3", "rect"))
hp2 = d.box("HP 40 Hz", 480, 530, 110, 45, AUD)
lp2 = d.box("LP COLOR", 620, 530, 110, 45, AUD)
f2 = d.box("f2", 760, 532, 50, 40, GAIN + "direction=west;")
t2 = d.box("tap2", 760, 470, 60, 40, AUD)
d.edge(r, s2, "in2")
d.edge(s2, h2)
d.edge(h2, t2)
d.edge(t2, hp2, "", EDGE, pts=((790, 520), (535, 520)))
d.edge(hp2, lp2)
d.edge(lp2, f2)
d.edge(f2, s2, "", EDGE, pts=((785, 590), (210, 590)))

# output gains
g = d.box("<b>output gains (g1, g2)</b><br>SINGLE (1, 0)<br>SERIES (1, 1)<br>PARALLEL (0.5, 0.5)", 930, 290, 190, 90, SW)
o = d.box("+", 1160, 315, 40, 40, SUM)
w = d.box("wet → mixer<br>y = x + MIX·wet", 1240, 305, 140, 60, HW)
d.edge(t1, g, "tap1", pts=((900, 140), (900, 320)))
d.edge(t2, g, "tap2", pts=((900, 490), (900, 350)))
d.edge(g, o)
d.edge(o, w)

# control law
d.box("<b>DigiMap — contract §5-6 control law</b> (once per block)<br>"
      "TIME slot → D1 = round(20 ms · 125<sup>t</sup> · 48 kHz), 20 ms … 2.5 s<br>"
      "SHIFT+K1 → r ∈ {1/4, 1/3, 3/8, 1/2, 2/3, 3/4, 1}, default 3/4<br>"
      "FEEDBACK → k = 0.95·knob (E1 and E2 linked); SHIFT+K2 → k2 alone; SHIFT+B1 relinks<br>"
      "SERIES: f = 1 − √(1 − k)   ·   SINGLE / PARALLEL: f = k<br>"
      "COLOR → LP from 0.49·fs (off) down to 2 kHz, log; HP fixed 40 Hz<br>"
      "MOTION: unassigned in DIGI · config change: next block, histories never cleared",
      930, 420, 470, 150, BLK + "align=left;spacingLeft=10;fillColor=#ffffff;strokeColor=#000000;")
d.box("Each engine per sample: tap = h[D] (read before write) → cond = LP(HP(tap)) → h ← in + f·cond. "
      "In SINGLE, E2 keeps running with input 0 and decays, so switching back to a dual config keeps its tail. "
      "Checks: test_digi (contract §10 markers, §6 build-up, 18 × 60 s soaks), test_parity (1 440 000 samples bit-exact vs DigitalDelayNode).",
      150, 620, 1250, 50, NOTE)

with open(OUT, "w", encoding="utf-8") as fh:
    fh.write('<mxfile host="Electron" type="device">' + s.xml() + d.xml() + "</mxfile>")
print("wrote", OUT)
