# EDGE Performance FX — User Manual

**Target hardware:** Electrosmith Daisy Pod + External I2C Control Board
**Signal chain:** Mono In → Input Conditioning → Soft Drive → Tempo-Synced Delay → Dark Diffusion → Wet/Dry → Output Tilt → Limiter → Dual-Mono Out
**Firmware:** `EDGE_mono_DSP` v1.0.0

---

## Table of Contents

1. [Hardware Overview](#1-hardware-overview)
2. [Control Reference](#2-control-reference)
3. [Signal Chain](#3-signal-chain)
4. [OLED Display](#4-oled-display)
5. [Pages](#5-pages)
   - [P1 — Perform](#p1--perform)
   - [P2 — Tone](#p2--tone)
   - [P3 — Motion](#p3--motion)
   - [P4 — Freeze](#p4--freeze)
   - [P5 — Presets](#p5--presets)
   - [P6 — System](#p6--system)
6. [Shift Layer](#6-shift-layer)
7. [Tap Tempo](#7-tap-tempo)
8. [Freeze](#8-freeze)
9. [Menu Navigation](#9-menu-navigation)
10. [Build & Flash](#10-build--flash)
11. [Factory Presets](#11-factory-presets)
12. [Parameter Reference](#12-parameter-reference)
13. [Functionality Test — Verification Through Presets](#13-functionality-test--verification-through-presets)

---

## 1. Hardware Overview

### Daisy Pod (native controls)

```
┌──────────────────────────────────────┐
│              DAISY POD               │
│                                      │
│   [KNOB 1]          [KNOB 2]         │
│   Delay Time        Feedback         │
│                                      │
│   [SW1]             [SW2]            │
│   Shift / Freeze    Tap Tempo        │
│                                      │
│        [POD ENCODER]                 │
│        Wet/Dry · Latch               │
│                                      │
│   AUDIO IN (L)   AUDIO OUT (L+R)     │
└──────────────────────────────────────┘
```

The Pod is wired for **mono-in / dual-mono-out**. Only the **Left input** is used. Both Left and Right outputs carry the identical processed signal.

### External I2C Control Board (connected via header pins)

```
┌──────────────────────────────────────┐
│          EXTERNAL BOARD              │
│                                      │
│         [EXT ENCODER]                │
│         Menu Navigate / Edit         │
│                                      │
│   [BAK]           [CON]              │
│   Back / Cancel   Confirm / Apply    │
│                                      │
│         [OLED 128×64]                │
│         SSD1306, I2C                 │
└──────────────────────────────────────┘
```

### Pin Assignments (External Board → Daisy Seed)

| Signal | Seed Pin | Wire Color (typical) | Function |
|--------|----------|---------------------|----------|
| TRA (Enc A) | D7 | White | Encoder quadrature A |
| TRB (Enc B) | D8 | Gray | Encoder quadrature B |
| PSH (Enc push) | D9 | Brown | Encoder push-button |
| CON | D10 | Green | Confirm button |
| BAK | D22 | Cherry | Back button |
| SCL (OLED) | D11 | Yellow | I2C clock |
| SDA (OLED) | D12 | Blue | I2C data |
| VCC | 3V3 | Red | 3.3 V power |
| GND | GND | Black | Ground |

> **Note:** D17 (LED2_R) is driven as a push-pull OUTPUT by the Pod's init routine. Do not wire any input signal to D17 or adjacent pins.

---

## 2. Control Reference

### Quick Map

| Control | Direct Action | Shift + Action |
|---------|--------------|----------------|
| **Knob 1** | Delay Time (subdivision snap or free ms) | Delay Subdivision select |
| **Knob 2** | Feedback (0–98%) | Feedback LP cutoff macro |
| **SW1 — short tap** | Freeze momentary (hold for freeze) | — |
| **SW1 — hold >200ms** | *Shift modifier active* | — |
| **SW2** | Tap Tempo | Delay Mode toggle (Sync ↔ Free) |
| **Pod Encoder — turn** | Wet/Dry mix | Drive amount |
| **Pod Encoder — push** | Freeze Latch toggle | Page Jump (P1→P2→P3→P4) |
| **Ext Encoder — turn** | Menu cursor up/down (or value change in edit mode) | — |
| **Ext PSH — short** | Enter/exit edit mode on selected item | — |
| **Ext PSH — hold 500ms** | Quick preset save (any page) | — |
| **BAK** | Exit edit mode / Return to P1 | — |
| **CON** | Confirm / enter edit mode | — |

### Knob 1 — Delay Time

In **Sync mode**: Knob 1 snaps across 10 beat divisions as you turn it.
The current subdivision is always shown on P1.

| Knob Zone | Division | Musical Name |
|-----------|----------|--------------|
| 0–10% | 1/16 | Sixteenth note |
| 10–20% | 1/16d | Dotted sixteenth |
| 20–30% | 1/16t | Sixteenth triplet |
| 30–40% | 1/8 | Eighth note |
| 40–50% | 1/8d | Dotted eighth |
| 50–60% | 1/8t | Eighth triplet |
| 60–70% | 1/4 | Quarter note (default) |
| 70–80% | 1/4d | Dotted quarter |
| 80–90% | 1/2 | Half note |
| 90–100% | 1/1 | Whole note |

In **Free mode**: Knob 1 sweeps continuously from 10 ms to 2000 ms.

### Knob 2 — Feedback

Controls the amount of delay signal fed back into the delay input.
- Range: 0% (single repeat) to 98% (near-infinite sustain)
- Hard-clamped at 98% to prevent runaway

### SW1 — Shift / Freeze

SW1 is a dual-purpose button — its behavior depends on how long it is held:

- **Tap quickly** (< 200 ms): engages **Freeze momentary** for the duration of the press. Release SW1 to unfreeze.
- **Hold > 200 ms**: activates **Shift modifier**. While Shift is active, Knob 1, Knob 2, Pod Enc turn, and SW2 all access their secondary functions. The OLED header shows `SHF`.

### SW2 — Tap Tempo / Delay Mode

- **Unshifted**: tap 2–4 times to set the BPM. The firmware calculates a rolling average of the last 4 inter-tap intervals. Taps more than 3 seconds apart reset the sequence.
- **Shift + SW2**: toggles Delay Mode between **SYNC** and **FREE**. In SYNC mode, delay time tracks Knob 1 subdivision at the current BPM. In FREE mode, Knob 1 controls delay time directly in milliseconds.

### Pod Encoder — Wet/Dry / Freeze Latch / Page Jump

- **Turn (unshifted)**: adjusts Wet/Dry mix.
- **Turn (Shift held)**: adjusts Drive amount.
- **Push (unshifted)**: toggles the Freeze Latch on or off.
- **Push (Shift held)**: Quick Page Jump — cycles P1 → P2 → P3 → P4 → P1.

---

## 3. Signal Chain

```
Audio In (Pod L)
    │
    ▼
[Input Trim]         — input_gain (0.5–2.0×)
    │
    ▼
[DC Blocker]         — removes DC offset automatically
    │
    ▼
[Input HP Filter]    — hp_hz (20–500 Hz) • P2
    │
    ▼
[Soft Drive]         — drive (0–100%) • Shift+Enc / P1
    │
    ▼
┌─────────────────────────────────────────┐
│           DELAY CORE (SDRAM)            │
│  ┌─────────────────────────────────┐    │
│  │         Feedback Path           │    │
│  │  FB HP → FB LP → Wow/Flutter    │    │
│  │  → optional FB Saturation       │    │
│  └─────────────────────────────────┘    │
└─────────────────────────────────────────┘
    │               │
    │ dry           │ wet
    ▼               ▼
    │         [Dark Diffusion]      — diffuse_damping • P2
    │               │
    └───────┬───────┘
            ▼
       [Wet/Dry Mix]               — wet (0–100%) • Pod Enc / P1
            │
            ▼
       [Output Tilt]               — output_tilt_db (±6 dB) • P2
            │
            ▼
         [Limiter]                 — hard ceiling protection
            │
            ▼
    Dual-Mono Out (Pod L + R)
```

> **Phase 1 note:** The DSP chain runs as audio passthrough while the delay engine, reverb, and tilt equalizer are being implemented. All controls update the parameter store and are visible on the OLED. DSP integration is Phase 2–4.

---

## 4. OLED Display

The OLED shows a header row followed by parameter rows for the current page.

### Header Row (top line, always inverted)

```
┌────────────────────────┐
│ PERFORM       SHF FRZ  │  ← inverted background
│ ...                    │
└────────────────────────┘
```

| Badge | Meaning |
|-------|---------|
| `SHF` | SW1 Shift is currently held |
| `FRZ` | Freeze is active (momentary or latched) |

### Parameter Rows

```
 Feedback         50%     ← normal row
>Wet/Dry Mix      40%     ← selected (highlighted, inverted)
[Drive            30%]    ← selected AND in edit mode (bracketed)
```

- **`>`** prefix: cursor is on this row (navigation mode)
- **`[ ]`** brackets: cursor on this row AND edit mode is active — turn Ext Enc to change value

---

## 5. Pages

Navigate between pages using **BAK** (return to P1) or **Shift + Pod Enc push** (cycle P1→P2→P3→P4).
Move the cursor with **Ext Encoder**. Enter edit mode with **Ext PSH** or **CON**. Exit with **BAK** or **Ext PSH** again.

---

### P1 — Perform

The default performance page. All live controls (knobs, SW1, SW2, Pod Enc) work from any page, but this page gives the clearest view of the primary parameters.

```
PERFORM        SHF FRZ
Time: 1/4 120BPM
>Feedback         50%
 Wet/Dry Mix      40%
 Drive             25%
 Freeze           OFF
─────────────────────
 SYNC  Sh+SW2=FREE
```

| Row | Parameter | Live Control | Edit via Ext Enc |
|-----|-----------|-------------|-----------------|
| Time | Delay subdivision + BPM | Knob 1 (subdiv), SW2 (tap BPM) | Step through subdivisions / adjust BPM |
| Feedback | Delay feedback | Knob 2 | ±2% per detent |
| Wet/Dry | Wet/dry mix | Pod Enc turn | ±2% per detent |
| Drive | Soft drive amount | Shift + Pod Enc turn | ±2% per detent |
| Freeze | Freeze state / mode | SW1 short (momentary), Pod Enc push (latch) | Toggle between Momentary and Latch modes |

**Bottom status line:** Shows current delay mode and the shortcut to toggle it.

---

### P2 — Tone

Filter and tonal shaping for every stage of the signal chain. All parameters edited via Ext Enc navigate + PSH.

**Exception:** Feedback LP also responds to **Shift + Knob 2** as a live macro from any page.

```
TONE
 Input HP       80Hz
>Feedback LP   6000Hz
 Feedback HP     60Hz
 Wet Tone      8000Hz
 Out Tilt       +0.0dB
─────────────────────
 Sh+K2=FB LP macro
```

| Row | Parameter | Range | Step | Function |
|-----|-----------|-------|------|----------|
| Input HP | HP filter cutoff on raw input | 20–500 Hz | 5 Hz | Removes low-end rumble before drive stage |
| Feedback LP | Low-pass in the feedback path | 500–18000 Hz | 200 Hz | Darkens repeats as they accumulate |
| Feedback HP | High-pass in the feedback path | 20–500 Hz | 5 Hz | Removes DC and bass buildup in feedback |
| Wet Tone | Damping LP on reverb/diffusion output | 1000–20000 Hz | 200 Hz | Controls darkness of the diffusion tail |
| Out Tilt | Shelving tilt EQ on final output | −6 to +6 dB | 0.5 dB | Positive = brighter, negative = warmer |

---

### P3 — Motion

Wow/flutter modulation applied to the delay tap read position. Creates subtle pitch warble at low depths — useful for vintage tape character.

```
MOTION
 Wow Depth        0%
>Wow Rate       1.2Hz
 Mod Enable      OFF
```

| Row | Parameter | Range | Step | Function |
|-----|-----------|-------|------|----------|
| Wow Depth | Modulation depth (LFO → delay tap offset) | 0–100% | 5% | 0% = no wow; higher = more pitch wobble |
| Wow Rate | LFO rate | 0.1–5.0 Hz | 0.1 Hz | Slow = gentle flutter; fast = vibrato-like |
| Mod Enable | Enable/disable modulation | ON / OFF | toggle | Master switch for all wow/flutter |

> Set Mod Enable to OFF to use the delay without any pitch modulation.

---

### P4 — Freeze

Detailed configuration for the freeze engine. The fast freeze controls (SW1 short, Pod Enc push) work from any page regardless of this page's settings.

```
FREEZE
>Mode        Momentary
 Behavior         Loop
 Loop Size       500ms
─────────────────────
 SW1=Momentary
 EncPush=Latch
```

| Row | Parameter | Options | Function |
|-----|-----------|---------|----------|
| Mode | Freeze engagement mode | Momentary / Latch | **Momentary**: freeze held while SW1 is pressed. **Latch**: Pod Enc push toggles freeze on/off permanently. |
| Behavior | What happens to the frozen buffer | Loop / Hold-last | **Loop**: repeats the frozen buffer in a continuous loop. **Hold-last**: holds the final state of the buffer without retriggering. |
| Loop Size | Size of the loop region | 50–2000 ms | 50 ms step. Only relevant in Loop mode. |

**Fast Freeze Controls (always active):**
- **SW1 short tap**: Freeze momentary — holds while SW1 is pressed, releases on release
- **Pod Enc push**: Freeze Latch — toggles freeze on/off. OLED shows `FRZ` badge when active.

---

### P5 — Presets

Load and save complete parameter snapshots. Five factory presets are included.

```
PRESETS
>P1: Kick Dub
 P2: Snare Space
 P3: Metal Burst
 P4: Noise Wash
 P5: Bass Echo Drv
```

**Controls:**
- **Ext Enc**: navigate to the desired preset slot
- **CON** or **Ext PSH**: load the highlighted preset
- **Ext PSH long (500 ms)** from any page: quick-save current parameters to the last-used slot

> **Phase 1 note:** Preset storage to flash memory is implemented in Phase 6. In Phase 1, presets initialize from factory values at boot.

---

### P6 — System

Hardware configuration and firmware information.

```
SYSTEM
 Brightness      75%
>Enc Dir         Normal
 Bypass           OFF
─────────────────────
 SR:48kHz  Blk:48
 EDGE FX  v1.0.0
```

| Row | Parameter | Options | Function |
|-----|-----------|---------|----------|
| Brightness | OLED display brightness | 25 / 50 / 75 / 100% | Reduces eye strain in dark environments |
| Enc Dir | External encoder direction | Normal / Flipped | Flip if encoder turns feel reversed on your unit |
| Bypass | Audio bypass | OFF / ON | Bypasses all DSP — dry audio passes through |

**Read-only info (bottom section):**
- Sample rate: 48 kHz
- Block size: 48 samples
- Firmware version: v1.0.0

---

## 6. Shift Layer

**SW1 held > 200 ms** activates the Shift modifier. The OLED header shows `SHF` while Shift is active.

While Shift is held, the following controls change function:

| Control | Normal Function | Shift Function |
|---------|----------------|----------------|
| Knob 1 | Delay Time (free ms if in Free mode) | Subdivision step (always snaps) |
| Knob 2 | Feedback | FB LP cutoff macro (500–18000 Hz) |
| SW2 | Tap Tempo | Delay Mode toggle (Sync ↔ Free) |
| Pod Enc turn | Wet/Dry mix | Drive amount |
| Pod Enc push | Freeze Latch toggle | Quick Page Jump |

### Notes on Shift behavior

- Shift activates **200 ms after SW1 is pressed**. This window prevents accidental Shift activation during a quick freeze tap.
- If you hold SW1 and touch another control, the Shift is "consumed" — releasing SW1 will **not** trigger a freeze momentary.
- If SW1 is pressed and released quickly (< 200 ms) without touching another control, it acts as a **freeze tap** only.

---

## 7. Tap Tempo

**SW2** is the Tap Tempo button.

- Tap **twice** to set a BPM (single interval).
- Tap **3–4 times** for a more accurate average (rolling 4-tap average).
- Taps more than **3 seconds apart** reset the sequence — the next tap starts a fresh measurement.
- BPM range: 40–240.

**Tip:** Tap along with the kick drum or click track before engaging the delay for perfectly locked rhythmic echoes.

**Shift + SW2** switches between **SYNC** and **FREE** mode without touching the BPM value:
- **SYNC**: delay time is computed as `(60000 ms / BPM) × subdivision_factor`. Automatically follows tempo changes.
- **FREE**: delay time is set directly by Knob 1 (10–2000 ms), ignoring BPM.

---

## 8. Freeze

The freeze engine captures the current delay buffer contents and holds them. Two modes of engagement:

### Momentary Freeze

1. Press **SW1 quickly** (short tap, < 200 ms).
2. The delay buffer is frozen while SW1 is held.
3. Release SW1 to resume normal delay operation.

Use this for stutter effects, momentary ambient pads, or rhythmic gating by rapidly tapping SW1.

### Latch Freeze

1. Press **Pod Encoder** to toggle freeze latch.
2. The OLED header shows **`FRZ`** badge while latched.
3. Press Pod Encoder again to release.

Use latch freeze to sustain a texture indefinitely while your hands are free for other controls.

### Freeze Mode Configuration (P4)

From **P4 — Freeze**, you can configure:
- **Mode** (Momentary / Latch): changes what Pod Enc push does
- **Behavior** (Loop / Hold-last): how the frozen content plays
- **Loop Size**: the region length for looped freeze

### Click Prevention

The firmware applies a short crossfade (5–10 ms) when engaging or releasing freeze to minimize audible clicks. For maximum smoothness, engage freeze on a quiet moment rather than a transient peak.

---

## 9. Menu Navigation

The Ext Encoder and its associated buttons form the complete menu navigation system.

### Normal Mode (not in edit)

| Action | Result |
|--------|--------|
| Ext Enc CW | Move cursor down (next parameter) |
| Ext Enc CCW | Move cursor up (previous parameter) |
| Ext PSH (short) | Enter **edit mode** on selected parameter |
| CON | Enter **edit mode** on selected parameter |
| BAK (short) | If on P1: nothing. If on P2-P6: return to P1 |
| Shift + Pod Enc push | Cycle pages P1→P2→P3→P4→P1 |

### Edit Mode (parameter selected for editing)

| Action | Result |
|--------|--------|
| Ext Enc CW | Increase value (step size depends on parameter) |
| Ext Enc CCW | Decrease value |
| Ext PSH (short) | Exit edit mode, confirm value |
| Ext PSH (long, 500ms) | Quick preset save, exit edit mode |
| CON | Exit edit mode, confirm value |
| BAK | Exit edit mode, **discard** change (note: parameter change is live, so value stays) |

> All parameter changes are **live** — you hear the effect immediately as you turn the encoder in edit mode. There is no "cancel" for audio parameters.

### Selecting a Page

To navigate to a specific page, use **Shift + Pod Enc push** to cycle forward through P1→P2→P3→P4, or use **BAK** from any page to return directly to P1. From P1, use Shift + Pod Enc push to jump to P2, etc.

There is no direct "page select" control by design — keeping the navigation shallow prevents accidental page switches during performance.

---

## 10. Build & Flash

### Prerequisites

- `arm-none-eabi-gcc` toolchain (tested with 10.x)
- Electrosmith `libDaisy` and `DaisySP` at `../../../` relative to this project
- `openocd` (for ST-Link flashing) or DFU USB support

### Build

**Default current path — Git Bash / POSIX shell**

```bash
cd DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP
make clean && make
```

**Codex current path — Git Bash with escalated permissions**

```powershell
& 'C:\Program Files\Git\bin\bash.exe' -lc "cd '/c/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP' && make clean && make"
```

**Codex fallback — PowerShell-safe rebuild**

```powershell
Set-Location 'DaisyExamples\MyProjects\_projects\POD_EDGE_mono_DSP'
if(Test-Path build){Remove-Item -LiteralPath build -Recurse -Force}
make
```

> `make clean` is implemented upstream in `libDaisy/core/Makefile` with `rm -fR build`, so use Git Bash for the clean target or the PowerShell fallback above when running through Codex.

Expected output (clean build):
```
Memory region   Used Size  Region Size  %age Used
       FLASH:     ~95 KB       128 KB     ~74%
        SRAM:     ~52 KB       512 KB     ~10%
```

### Flash via ST-Link

```bash
make program
```

**Codex / PowerShell**

```powershell
Set-Location 'DaisyExamples\MyProjects\_projects\POD_EDGE_mono_DSP'
make program
```

ST-Link must be connected and recognized by your OS. On Windows, use [Zadig](https://zadig.akeo.ie/) to install the **WinUSB** driver for `STM32 STLink`.

### Flash via DFU (USB)

If ST-Link is unavailable:

1. Hold the **BOOT** button on the Daisy Seed
2. While holding BOOT, press and release **RESET**
3. Release BOOT — the device enters DFU mode
4. Run: `make program-dfu`

**Codex / PowerShell**

```powershell
Set-Location 'DaisyExamples\MyProjects\_projects\POD_EDGE_mono_DSP'
make program-dfu
```

### Verify

After flashing, the OLED should display `EDGE FX` for ~2 seconds, then switch to the P1 Perform page. If the display stays blank, check the I2C wiring (SCL=D11, SDA=D12) and the I2C pull-up resistors on the external board.

---

## 11. Factory Presets

Five presets voiced specifically for EDGE/Behringer EDGE-style analog material:

### P1: Kick Dub

Punchy sub-bass echo with filtered repeats that decay naturally. Short delay (1/8d at 120 BPM), low feedback, warm LP on repeats. Adds weight without muddying the kick transient.

### P2: Snare Space

Wide snare reinforcement with a 1/4 note echo and slight diffusion. Moderate feedback with high-frequency damping so repeats feel natural, not digital. Works on both rimshots and snare hits.

### P3: Metal Burst

Short 1/16 delay with high feedback and bright, harsh repeats. Designed for metallic percussion, rim hits, and industrial noise bursts. Output tilt slightly bright (+1.5 dB) to preserve attack.

### P4: Noise Wash

Long diffuse tail (1/2 note, high wet, heavy reverb damping). Turns noise bursts and filtered sweeps into sustained ambient textures. Pairs well with freeze latch.

### P5: Bass Echo Drive

Driven bass echo — input drive at 65%, low feedback, short 1/8 delay at moderate wet. Gives bass pulses and sequences a saturated, punchy double. High-pass at 80 Hz keeps the low-end tight.

---

## 12. Parameter Reference

Complete list of all parameters, their ranges, assigned controls, and which page exposes them for editing.

### P1 — Perform

| Parameter | `FxParams` field | Range | Live Control | Edit Step |
|-----------|-----------------|-------|-------------|-----------|
| Delay Subdivision | `subdiv_idx` | 1/16 → 1/1 (10 steps) | Knob 1 / Shift+Knob 1 | 1 step per detent |
| Delay Time (free) | `delay_time_ms` | 10–2000 ms | Knob 1 (free mode) | 10 ms per detent |
| BPM | `bpm` | 40–240 | SW2 (tap) | — |
| Delay Mode | `sync_mode` | Sync / Free | Shift + SW2 | toggle |
| Feedback | `feedback` | 0–98% | Knob 2 | 2% per detent |
| Wet/Dry Mix | `wet` | 0–100% | Pod Enc / Ext Enc | 2% per detent |
| Drive | `drive` | 0–100% | Shift + Pod Enc | 2% per detent |
| Freeze (momentary) | `freeze_momentary` | ON while held | SW1 short | — |
| Freeze (latch) | `freeze_latched` | ON / OFF | Pod Enc push | toggle |
| Freeze Mode | `freeze_latch_mode` | Momentary / Latch | Ext Enc (edit) | toggle |

### P2 — Tone

| Parameter | `FxParams` field | Range | Live Control | Edit Step |
|-----------|-----------------|-------|-------------|-----------|
| Input HP cutoff | `hp_hz` | 20–500 Hz | — | 5 Hz per detent |
| Feedback LP cutoff | `fb_lp_hz` | 500–18000 Hz | Shift + Knob 2 (macro) | 200 Hz per detent |
| Feedback HP cutoff | `fb_hp_hz` | 20–500 Hz | — | 5 Hz per detent |
| Wet tone / damping | `diffuse_damping` | 1000–20000 Hz | — | 200 Hz per detent |
| Output tilt | `output_tilt_db` | −6 to +6 dB | — | 0.5 dB per detent |

### P3 — Motion

| Parameter | `FxParams` field | Range | Live Control | Edit Step |
|-----------|-----------------|-------|-------------|-----------|
| Wow depth | `wow_depth` | 0–100% | — | 5% per detent |
| Wow rate | `wow_rate_hz` | 0.1–5.0 Hz | — | 0.1 Hz per detent |
| Mod enable | `wow_enabled` | ON / OFF | — | toggle |

### P4 — Freeze

| Parameter | `FxParams` field | Range | Live Control | Edit Step |
|-----------|-----------------|-------|-------------|-----------|
| Freeze mode | `freeze_latch_mode` | Momentary / Latch | — | toggle |
| Hold behavior | `freeze_loop_mode` | Loop / Hold-last | — | toggle |
| Loop size | `freeze_size_ms` | 50–2000 ms | — | 50 ms per detent |

### P6 — System

| Parameter | `FxParams` field | Range | Live Control | Edit Step |
|-----------|-----------------|-------|-------------|-----------|
| OLED brightness | — | 25 / 50 / 75 / 100% | — | 25% per detent |
| Encoder direction | `enc_flipped` | Normal / Flipped | — | toggle |
| Bypass | `bypass` | OFF / ON | — | toggle |

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────┐
│                 EDGE FX — QUICK REFERENCE                │
├──────────────┬──────────────────────────────────────────┤
│  K1          │ Delay Time / Subdivision                  │
│  K2          │ Feedback (0–98%)                          │
│  SW1 tap     │ FREEZE momentary                          │
│  SW1 hold    │ SHIFT modifier active                     │
│  SW2         │ TAP TEMPO                                 │
│  Pod Enc →   │ WET/DRY mix                               │
│  Pod Enc ●   │ FREEZE LATCH toggle                       │
├──────────────┼──────────────────────────────────────────┤
│  Shift+K1    │ Subdivision select                        │
│  Shift+K2    │ Feedback LP macro                         │
│  Shift+SW2   │ SYNC ↔ FREE mode toggle                   │
│  Shift+Enc→  │ DRIVE amount                              │
│  Shift+Enc●  │ PAGE JUMP P1→P2→P3→P4                    │
├──────────────┼──────────────────────────────────────────┤
│  Ext Enc →   │ Navigate / adjust value (edit mode)       │
│  Ext PSH     │ Edit mode on/off                          │
│  BAK         │ Exit edit / Return to P1                  │
│  CON         │ Confirm / enter edit                      │
│  Ext PSH 5s  │ Quick preset save                         │
├──────────────┼──────────────────────────────────────────┤
│  PAGES       │ P1=Perform P2=Tone P3=Motion              │
│              │ P4=Freeze  P5=Presets P6=System           │
└──────────────┴──────────────────────────────────────────┘
```

---

## 13. Functionality Test — Verification Through Presets

A systematic test procedure using the 10 built-in presets to verify that every subsystem of the firmware is functioning correctly. Run this checklist after initial flash or after any firmware update.

**How to load a preset:** Navigate to P5 (Presets) by scrolling the Ext Encoder past the last item on any page, or by pressing Shift + Pod Enc push until PRESETS appears in the header. Scroll to the desired slot, then press **Ext PSH** or **CON** to load. The display returns to P1 automatically showing the loaded parameter values.

**Pass/Fail convention:**
- ✅ **PASS** — observed behaviour matches the description
- ❌ **FAIL** — something is wrong; note the symptom and consult the debug notes

---

### Test 0 — Boot Sequence

**Before loading any preset, verify the device boots correctly.**

| Step | Action | Expected | Result |
|------|--------|----------|--------|
| 0-A | Power on (USB or VIN) | OLED shows "EDGE FX" for ~2 seconds, then transitions to P1 Perform | ☐ |
| 0-B | Observe P1 | Header shows `PERFORM`, no `SHF` or `FRZ` badge, Time shows `1/4 120BPM` | ☐ |
| 0-C | Plug audio cable into L input, monitoring on L+R output | No noise, no hum (Phase 1: passthrough = dry signal through) | ☐ |

**If OLED stays blank:** Check SCL=D11, SDA=D12 wiring and I2C pull-up resistors.

---

### Test 1 — Page Navigation (no preset needed)

**Verifies that all 6 pages are reachable via Ext Encoder scroll.**

| Step | Action | Expected |
|------|--------|----------|
| 1-A | Starting on P1, turn Ext Enc CW past last item (Freeze row) | OLED header changes to `TONE`, cursor on first item | ☐ |
| 1-B | Continue scrolling CW past last item on P2 | Header changes to `MOTION` | ☐ |
| 1-C | Continue scrolling CW past last item on P3 | Header changes to `FREEZE` | ☐ |
| 1-D | Continue scrolling CW past last item on P4 | Header changes to `PRESETS` with scroll indicator `  1 / 10` | ☐ |
| 1-E | Continue scrolling CW past preset 10 | Header changes to `SYSTEM` | ☐ |
| 1-F | Continue scrolling CW past last item on P6 | Wraps back to `PERFORM` (P1) | ☐ |
| 1-G | From P1, press **Shift + Pod Enc push** 6 times | Cycles P1→P2→P3→P4→P5→P6→P1 | ☐ |
| 1-H | From any page, press **BAK** | Returns immediately to `PERFORM` | ☐ |

---

### Test 2 — Control Response (no preset needed)

**Verifies that live controls update displayed values.**

| Step | Action | Expected |
|------|--------|----------|
| 2-A | Turn Knob 1 from min to max on P1 | Time display cycles through `1/16` → `1/16d` → … → `1/1` (10 steps) | ☐ |
| 2-B | Turn Knob 2 from min to max | Feedback display changes from `0%` to `98%` | ☐ |
| 2-C | Turn Pod Encoder CW | Wet/Dry display increases toward `100%` | ☐ |
| 2-D | Turn Pod Encoder CCW | Wet/Dry display decreases toward `0%` | ☐ |
| 2-E | Hold SW1 >200 ms | `SHF` badge appears in header | ☐ |
| 2-F | While holding SW1, turn Knob 2 | Feedback LP value on P2 changes (Shift+K2 macro) | ☐ |
| 2-G | Release SW1 | `SHF` badge disappears | ☐ |
| 2-H | Navigate to P2, select `Out Tilt` row, press Ext PSH, turn Ext Enc CW | Tilt value increases from `+0.0dB` toward `+6.0dB` in 0.5 dB steps | ☐ |
| 2-I | Press Ext PSH again | Edit mode exits (brackets removed) | ☐ |

---

### Test 3 — Tap Tempo

**Verifies the rolling-average BPM calculation.**

| Step | Action | Expected |
|------|--------|----------|
| 3-A | On P1, tap SW2 once | No BPM change (single tap, no interval to measure) | ☐ |
| 3-B | Tap SW2 twice at ~120 BPM rate (0.5 s apart) | BPM display updates to approximately `120` | ☐ |
| 3-C | Tap SW2 four times at a consistent rate | BPM display stabilises (rolling average smooths out) | ☐ |
| 3-D | Wait 4+ seconds, then tap once | BPM does not change (tap sequence resets on 3 s gap; single tap provides no interval) | ☐ |
| 3-E | Tap at 80 BPM rate (0.75 s apart) | BPM display updates to approximately `80` | ☐ |

---

### Test 4 — Preset Load: P06 Init Check

**Verifies clean passthrough — the reference baseline for all audio tests.**

**Load:** Navigate to P5 → P06 (`T:InitChek`) → press CON or Ext PSH.

Expected values visible on P1 after load:

| Parameter | Expected Value |
|-----------|---------------|
| Time | `1/4  120BPM` (sync mode, 1/4 subdivision) |
| Feedback | `0%` |
| Wet/Dry | `0%` |
| Drive | `0%` |
| Freeze | `OFF` |

| Step | Action | Expected |
|------|--------|----------|
| 4-A | Send audio into L input | Output L and R are identical to input — no delay, no colouration | ☐ |
| 4-B | Check P2 values | `Input HP: 20Hz`, `FB Lo-P: 18000Hz`, `Out Tilt: +0.0dB` (all effectively bypassed) | ☐ |
| 4-C | Mute input | Output goes silent (no delay tail, no reverb ring) | ☐ |

**Debug note:** If output sounds different from input in Phase 1 (passthrough firmware), check the AudioCallback — it should be a straight `out[0][i] = out[1][i] = in[0][i]` with `bypass=false` and `wet=0`.

---

### Test 5 — Preset Load: P07 Max Feedback

**Verifies feedback parameter range and stability at near-maximum.**

**Load:** P5 → P07 (`T:MaxFeedb`) → CON.

| Parameter | Expected on P1 |
|-----------|---------------|
| Feedback | `95%` |
| Wet/Dry | `40%` |
| Time | `1/4  100BPM` |

| Step | Action | Expected |
|------|--------|----------|
| 5-A | Send a single percussive hit | (Phase 1: no actual delay yet — note expected DSP behaviour for Phase 2+) Delay repeats sustain many seconds without runaway or distortion | ☐ |
| 5-B | Verify P1 Feedback row shows `95%` | Confirms preset loaded correctly | ☐ |
| 5-C | Turn Knob 2 to maximum | Feedback display reads `98%` (hard clamp — cannot exceed 98% regardless of knob position) | ☐ |

---

### Test 6 — Preset Load: P08 Wow Deep

**Verifies the wow/flutter engine parameters are loaded correctly.**

**Load:** P5 → P08 (`T:WowDeep`) → CON.

| Step | Action | Expected |
|------|--------|----------|
| 6-A | Navigate to P3 (Motion) | `Wow Depth: 80%`, `Wow Rate: 2.0Hz`, `Mod Enable: ON` | ☐ |
| 6-B | Navigate back to P1 | `Wow Deep` preset values visible on performance page | ☐ |
| 6-C | On P3, select `Mod Enable`, enter edit, turn Ext Enc to `OFF` | `Mod Enable` changes to `OFF` | ☐ |
| 6-D | Turn Ext Enc to `ON` again, exit edit | `Mod Enable` returns to `ON` | ☐ |
| 6-E | Select `Wow Rate`, enter edit, turn Ext Enc CW | Rate increases in 0.1 Hz steps | ☐ |
| 6-F | Turn Ext Enc CCW below 0.1 Hz | Rate clamps at `0.1Hz` (minimum) | ☐ |

**Debug note:** Phase 1 firmware has no Phasor running — wow is a parameter store test only. Audible pitch wobble requires Phase 3 DSP integration.

---

### Test 7 — Preset Load: P09 Full Drive

**Verifies drive and input gain parameters load and display correctly.**

**Load:** P5 → P09 (`T:FullDriv`) → CON.

| Parameter | Expected on P1 |
|-----------|---------------|
| Drive | `100%` |
| Wet/Dry | `25%` |

| Step | Action | Expected |
|------|--------|----------|
| 7-A | Confirm P1 shows `Drive: 100%` | Preset loaded correctly | ☐ |
| 7-B | Hold Shift + turn Pod Enc CCW | Drive decreases from `100%` | ☐ |
| 7-C | Hold Shift + turn Pod Enc CW | Drive increases back toward `100%`, clamps at `100%` | ☐ |
| 7-D | Navigate to P2, check `Out Tilt` | Should show `-1.0dB` (preset value) | ☐ |

---

### Test 8 — Preset Load: P10 Freeze Pad

**Verifies freeze parameter loading, latch mode, and all freeze controls.**

**Load:** P5 → P10 (`T:FreezePd`) → CON.

| Parameter | Expected |
|-----------|---------|
| Wet/Dry | `65%` |
| P4 Mode | `Latch` |
| P4 Behavior | `Loop` |
| P4 Loop Size | `800ms` |

| Step | Action | Expected |
|------|--------|----------|
| 8-A | Navigate to P4 (Freeze) | Mode: `Latch`, Behavior: `Loop`, Loop Size: `800ms` | ☐ |
| 8-B | Return to P1, press Pod Enc push | `FRZ` badge appears in header, Freeze row shows `LATCH` | ☐ |
| 8-C | Press Pod Enc push again | `FRZ` badge disappears, Freeze row shows `OFF` | ☐ |
| 8-D | Press SW1 quickly (short tap) | Freeze momentary: `FRZ` appears while held | ☐ |
| 8-E | Release SW1 | `FRZ` disappears | ☐ |
| 8-F | On P4, change Mode to `Momentary`, return to P1, press Pod Enc push | Pod Enc push now toggles momentary (latch_mode=false behaviour) | ☐ |

---

### Test 9 — Shift Badge Visibility

**Verifies the SHF status badge appears and disappears correctly.**

| Step | Action | Expected |
|------|--------|----------|
| 9-A | Press and immediately release SW1 | No `SHF` badge (hold too short) | ☐ |
| 9-B | Press and hold SW1 for 300 ms | `SHF` badge appears in header top-right | ☐ |
| 9-C | While holding SW1 >200 ms, turn Knob 2 | Feedback LP value on P2 changes (Shift+K2 macro confirmed active) | ☐ |
| 9-D | Release SW1 | `SHF` badge disappears | ☐ |
| 9-E | Press and hold SW1 >200 ms, press Pod Enc push | Page advances one step (PAGE_JUMP); releasing SW1 does NOT trigger freeze | ☐ |

---

### Test 10 — Full Preset Cycle

**Verifies all 10 preset slots are accessible and load without hanging.**

| Step | Action | Expected |
|------|--------|----------|
| 10-A | Navigate to P5 | PRESETS header, P01 `KickDub` at top, scroll indicator `  1 / 10` | ☐ |
| 10-B | Scroll CW through all 10 presets | Each slot name appears correctly; indicator shows ` 1/10` through `10/10` | ☐ |
| 10-C | Load each preset (CON or Ext PSH) one by one | Display returns to P1 after each load; no freeze or hang | ☐ |
| 10-D | After loading P01 Kick Dub, check P1 values | `Time: 1/8d 120BPM`, `Feedback: 40%`, `Wet/Dry: 45%` | ☐ |
| 10-E | After loading P04 Noise Wash, check P1 values | `Feedback: 72%`, `Wet/Dry: 70%` | ☐ |
| 10-F | After loading P06 Init Check, check P1 values | `Feedback: 0%`, `Wet/Dry: 0%`, `Drive: 0%` | ☐ |

---

### Test Summary Checklist

| Test | Subsystem | Status |
|------|-----------|--------|
| 0 | Boot sequence + OLED init | ☐ |
| 1 | Page navigation (all 6 pages) | ☐ |
| 2 | Live controls (K1, K2, SW1 Shift, Pod Enc, Ext Enc edit) | ☐ |
| 3 | Tap tempo (rolling average, reset on gap) | ☐ |
| 4 — P06 | Clean passthrough / Init Check | ☐ |
| 5 — P07 | Feedback clamp at 98% | ☐ |
| 6 — P08 | Wow/Flutter parameter loading and editing | ☐ |
| 7 — P09 | Drive control (Shift+Enc) and parameter clamp | ☐ |
| 8 — P10 | Freeze latch, momentary, FRZ badge | ☐ |
| 9 | SHF badge, Shift consume logic | ☐ |
| 10 | All 10 presets load without error | ☐ |

All ✅ = firmware Phase 1 verified and ready for Phase 2 DSP integration.

---

*EDGE FX User Manual · v1.0.0 · Hardware MIDI not implemented in v1 (see SOW §3.3)*
