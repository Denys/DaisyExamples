# Pod MultiDelay v0.5 controls

Delay pages: the encoder turns through DIGI / TAPE / MOD / REV / FREEZE; a click
cycles TIME/FEEDBACK -> MIX/COLOR -> MOTION (DIGI: E2 ratio view) -> TIME/FEEDBACK.
DRIVE is no longer in the click cycle. Press B1 and B2 together to enter DRIVE from
any delay page; press them together again to return to the page you came from.

"Together" means both buttons go down within 80 ms (PROPOSED window, tune on the
Pod). Because of that window, B1 alone acts 80 ms after the press, or on release if
released sooner. B2 held longer than 80 ms before B1 is SHIFT+B1, as in v0.4.

| Control | DRIVE | Delay pages |
|---|---|---|
| B1 + B2 together | Exit to the previous delay page | Enter DRIVE |
| Encoder turn | Type: OVERDRIVE / DISTORTION / FUZZ | Delay mode |
| B2 + encoder turn | Type (same as turn) | DIGI config / FREEZE operation |
| Encoder click | GAIN/TONE <-> LEVEL/BLEND | Next delay page |
| B2 + encoder click | Trails on/off | Trails on/off |
| K1 | GAIN or LEVEL | Page assignment |
| K2 | TONE or BLEND | Page assignment |
| B1 alone | DRIVE on/off | Delay bypass |
| B2 short release | Tap tempo / FREEZE capture-release | Same |
| B2 + B1 (B2 first, > 80 ms) | DIGI relink / FREEZE clear | Same |

DRIVE stays as you left it (on/off, type, settings) across pages, modes, delay
bypass and reconfiguration pauses. Each type keeps its own GAIN, TONE, LEVEL and
BLEND. Reboot restores OFF, DISTORTION, defaults per type.

| Type | GAIN range | Pre high-pass | Clipper | Default LEVEL |
|---|---|---|---|---|
| OVERDRIVE | 0..+30 dB | 720 Hz | tanh soft clip | 0 dB |
| DISTORTION | +6..+50 dB | 150 Hz | v0.4 cubic law | -6 dB |
| FUZZ | +20..+66 dB | 80 Hz | asymmetric hard clip [-0.6, +1], gate | -9 dB |

TONE: one-pole low-pass 1..12 kHz. LEVEL: -24..+6 dB. BLEND: clean..driven.
Clipping uses first-order antiderivative anti-aliasing (ADAA); host measurement at
maximum gain: alias-to-harmonic ratio OD -22.6 -> -29.5 dB, DIST -15.7 -> -25.6 dB,
FUZZ -15.5 -> -25.6 dB. A type change mutes to clean for 5 ms, switches, fades back.
The FUZZ gate (PROPOSED) opens above -54 dBFS and closes below -60 dBFS at the input.

## OLED (SSD1306 / SSD1315, 128x64, I2C 0x3C)

Wiring: SCL to Seed D11 (PB8), SDA to D12 (PB9), VCC 3V3, GND. I2C1 at 400 kHz. The
firmware runs normally without a display and retries every 2 s.

Delay screen: header with mode, DIGI config or FREEZE state, page P1/3; K1 and K2
names with values in musician units and a bar each; status line with bypass, drive
type and state, trails. Holding B2 in DIGI shows the E2 RATIO and E2 FEEDBACK that
K1/K2 edit. DRIVE screen: header with GAIN or LEVEL page and ON/OFF, type name in
large letters, two parameter bars. A short notice box confirms trails, tap, relink,
FREEZE clear, bypass, DRIVE on/off and DRIVE entry/exit.

Pickup: until a knob moves, its bar shows a hollow tick at the knob's physical
position next to the stored value; moving the knob 1.2 % takes over the value.

LED1 keeps the delay mode colour (dim in bypass, pulsing while DRIVE is on). LED2
is yellow inside DRIVE (bright on, dim off), red during a reconfiguration pause, and
the page colour otherwise.
