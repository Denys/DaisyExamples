# Field Template June

`Field_Template_June` is a controls-optimized Daisy Field starter synth derived
from `Field_Template_April`. It keeps the same small external-MIDI mono synth
role, but changes the control runtime so the Field surface stays stable after
bank, modifier, reset, or hidden-page changes while OLED and LED work no longer
runs every 1 ms.

The main control difference is the "until touched" algorithm. April used
pickup/catch: after switching from `K1` to shifted `K1`, a stored value became
editable only when the physical knob moved near the stored parameter value.
June instead records the physical knob position at bank/modifier entry and
keeps the stored value active until that knob moves by more than `0.012`. SW1
is a toggle now, not a hold modifier: press SW1 once to edit the alt bank, press
it again to return to the main bank. This means alt `K1` cannot change plain
`K1`, and plain `K1` becomes editable again as soon as `K1` moves after
returning to the main bank.

## Runtime Improvements

| Area | April behavior | June behavior |
|---|---|---|
| Banked knobs | Pickup/catch near stored value, threshold `0.02` | Until-touched movement gate from entry anchor, threshold `0.012` |
| Hidden level | `SW2 + K8` also used pickup/catch | Hidden level has its own entry anchor and touch gate |
| OLED | Redrawn every 1 ms main-loop pass | Dirty/capped redraw, max 20 Hz |
| LEDs | Key/knob LED transmit every 1 ms | LED transmit capped to about 60 Hz |
| Voice setup | Re-applied every main-loop pass | Applied only after parameter/state changes |
| MIDI queue | Drained without an explicit per-tick cap | Handles up to 16 events per 1 ms tick |
| Startup | ADC started before audio | Startup OLED, ADC prime pass, 600 ms settle, then audio |

## Controls

| Control | Role | Notes |
|---|---|---|
| `K1` | `Cutoff` | Main bank |
| `K2` | `Reso` | Main bank |
| `K3` | `Attack` | Main bank |
| `K4` | `Decay` | Main bank |
| `K5` | `Sustain` | Main bank |
| `K6` | `Release` | Main bank |
| `K7` | `Drive` | Main bank |
| `K8` | `Color` | Main bank; output level is intentionally hidden |
| `SW1` | Toggle shift bank | `MAIN` / `ALT`; shown on OLED and SW1 LED |
| `K1` in `ALT` | `EnvAmt` | Alt bank after SW1 toggle |
| `K2` in `ALT` | `LfoRt` | Alt bank after SW1 toggle; `0.1-12 Hz`, displayed in mHz below `1 Hz` |
| `K3` in `ALT` | `LfoDp` | Alt bank after SW1 toggle |
| `K4` in `ALT` | `Glide` | Alt bank after SW1 toggle |
| `K5` in `ALT` | `VelAmt` | Alt bank after SW1 toggle |
| `K6` in `ALT` | `KeyTrk` | Alt bank after SW1 toggle |
| `K7` in `ALT` | `Noise` | Alt bank after SW1 toggle |
| `K8` in `ALT` | `Sub` | Alt bank after SW1 toggle |
| `SW2` | Toggle B row mode | `CONTROLS` / `PERFORMANCE`; shown on OLED and SW2 LED |
| `SW2 + K8` | `Level` | Hidden output trim while SW2 is held, with its own touch gate |
| `A1-A4` | Waveform select | `SINE`, `TRI`, `SAW`, `SQR` |
| `A5` | Velocity mode | Cycles `FIX` -> `SCL` -> `PCH` |
| `A6` | Key tracking mode | Cycles `OFF` -> `HALF` -> `FULL` |
| `A7` | LFO target | Cycles `OFF` -> `PITCH` -> `FILT` |
| `A8` | Glide mode | Cycles `OFF` -> `LEG` -> `ON` |
| `B1-B4` | MIDI transpose select | `CONTROLS` mode: `-12`, `0`, `+12`, `+24` semitones |
| `B5` | Panic | `CONTROLS` mode: all notes off |
| `B6` | Reset main bank | `CONTROLS` mode: restores main defaults and records new touch anchors |
| `B7` | Reset alt bank | `CONTROLS` mode: restores alt defaults and records new touch anchors |
| `B8` | Reset all | `CONTROLS` mode: restores all defaults and records new touch anchors |
| `B1-B8` | Test note keyboard | `PERFORMANCE` mode: `C4 D4 E4 F4 G4 A4 B4 C5` |
| MIDI input | Note control | `Note On`, `Note Off`, velocity, `CC64` sustain |

## LED States

Knob LEDs show stored logical values, not raw potentiometer positions. A bright
knob LED means that parameter has been touched in the current bank/modifier
context and is live. A dim knob LED means a stored value exists but the physical
knob has not moved since entering that context. While holding `SW2`, `K8` shows
the hidden output level and its own live/touched state.

The Field key LEDs stay quiet by default. Active selection groups remain lit:
`A1-A4` show waveform selection and `B1-B4` show transpose selection while the
B row is in `CONTROLS` mode. `A5-A8` are tri-state mode indicators: the June
default is off, the next mode step blinks, and the second mode step is solid on.
`B5-B8` utility keys stay off until used. In `PERFORMANCE` mode, the B-row LEDs
show the currently held test notes instead of transpose/reset controls. The SW2
switch LED is on when B-row `PERFORMANCE` mode is active. The SW1 switch LED is
on while the `ALT` bank is active.

## OLED Pages

The startup page identifies the June template and reports that the ADC/UI state
is being primed. After audio starts, the normal overview is shown unless a
parameter or state has just changed.

```text
JUNE CONTROLS|PERFORMANCE
MAIN|ALT WAVE  TR:TRANSPOSE
A:VEL KEY LFO GLIDE
P1:VALUE   P5:VALUE
P2:VALUE   P6:VALUE
P3:VALUE   P7:VALUE
P4:VALUE   P8:VALUE
```

The edit zoom appears for about `1.4 s` after a parameter or state change and
is redrawn at most every `50 ms`. OLED value formatting intentionally uses
integer-only `snprintf` formats because the Make-based Daisy firmware links
with newlib-nano and does not enable float printf support. LFO rate is the only
parameter that uses mHz for sub-Hz display; the lowest `0.1 Hz` value is shown
as `100 mHz`.

```text
EDIT MAIN|ALT
PARAMETER NAME
LARGE VALUE TEXT
SW2:CONTROLS|PERFORMANCE
```

## Build And Flash

The visual source project is [Field_Template_June.dvpe](Field_Template_June.dvpe).
It mirrors the firmware block structure, hardware target, sample rate, block
size, parameter ranges/defaults, and Field control bindings. Update it with the
source whenever DSP structure, control ownership, parameter ranges, or defaults
change.

Build from the project directory:

```powershell
make
```

Flash through ST-Link from the project directory:

```powershell
make program
```

The ST-Link path avoids manually setting DFU state. Hardware flashing is not
part of normal build validation; use the hardware test plan when the Field is
connected.

## Validation

Current software validation on 2026-06-07 after lowering LFO minimum to
`0.1 Hz`, adding sub-Hz mHz display for LFO rate, and adding the `.dvpe`
visual source project:

| Check | Result |
|---|---|
| `make` | PASS |
| `Field_Template_June.dvpe` JSON parse | PASS |
| Daisy QAE validator | PASS, `0 error(s), 0 warning(s)` |
| ST-LINK flash | PASS, OpenOCD programmed, verified OK, and reset target on 2026-06-07 |
| Hardware run observation | Not manually observed after reset |

Current build footprint:

| Region | Used | Capacity | Usage |
|---|---:|---:|---:|
| FLASH | `130424 B` | `128 KB` | `99.51%` |
| SRAM | `64912 B` | `512 KB` | `12.38%` |
| RAM_D2 | `17224 B` | `288 KB` | `5.84%` |
| SDRAM | `0 B` | `64 MB` | `0.00%` |

## Maintenance Notes

Keep the synth intentionally small. This project is meant to be a reliable
Field control template, not a flagship instrument. Preserve the separation
between 1 ms control processing, capped visual refresh, and audio-only DSP
work. If additional banks or hidden pages are added, give each context its own
touch anchor so parameter values cannot jump when controls are reused.
