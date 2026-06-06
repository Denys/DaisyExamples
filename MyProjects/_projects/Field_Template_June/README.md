# Field Template June

`Field_Template_June` is a controls-optimized Daisy Field starter synth derived
from `Field_Template_April`. It keeps the same small external-MIDI mono synth
role, but changes the control runtime so the Field surface stays stable after
bank, modifier, reset, or hidden-page changes while OLED and LED work no longer
runs every 1 ms.

The main control difference is the "until touched" algorithm. April used
pickup/catch: after switching from `K1` to `SW1 + K1`, a stored value became
editable only when the physical knob moved near the stored parameter value.
June instead records the physical knob position at bank/modifier entry and
keeps the stored value active until that knob moves by more than `0.012`. This
means `SW1 + K1` cannot change plain `K1`, and plain `K1` becomes editable
again as soon as `K1` moves after returning to the main bank.

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
| `SW1 + K1` | `EnvAmt` | Alt bank |
| `SW1 + K2` | `LfoRt` | Alt bank |
| `SW1 + K3` | `LfoDp` | Alt bank |
| `SW1 + K4` | `Glide` | Alt bank |
| `SW1 + K5` | `VelAmt` | Alt bank |
| `SW1 + K6` | `KeyTrk` | Alt bank |
| `SW1 + K7` | `Noise` | Alt bank |
| `SW1 + K8` | `Sub` | Alt bank |
| `SW2 + K8` | `Level` | Hidden output trim with its own touch gate |
| `A1-A4` | Waveform select | `SINE`, `TRI`, `SAW`, `SQR` |
| `A5` | Velocity mode | Cycles `FIX` -> `SCL` -> `PCH` |
| `A6` | Key tracking mode | Cycles `OFF` -> `HALF` -> `FULL` |
| `A7` | LFO target | Cycles `OFF` -> `PITCH` -> `FILT` |
| `A8` | Glide mode | Cycles `OFF` -> `LEG` -> `ON` |
| `B1-B4` | MIDI transpose select | `-12`, `0`, `+12`, `+24` semitones |
| `B5` | Panic | All notes off |
| `B6` | Reset main bank | Restores main defaults and records new touch anchors |
| `B7` | Reset alt bank | Restores alt defaults and records new touch anchors |
| `B8` | Reset all | Restores all defaults and records new touch anchors |
| MIDI input | Note control | `Note On`, `Note Off`, velocity, `CC64` sustain |

## LED States

Knob LEDs show stored logical values, not raw potentiometer positions. A bright
knob LED means that parameter has been touched in the current bank/modifier
context and is live. A dim knob LED means a stored value exists but the physical
knob has not moved since entering that context. While holding `SW2`, `K8` shows
the hidden output level and its own live/touched state.

The Field key LEDs remain a state display. `A1-A4` and `B1-B4` are one-hot
selectors. `A5-A8` show tri-state modes with off, blink, and on states. `B5`
shows the current gate/sustain state, while `B6-B8` mark reset helpers.

## OLED Pages

The startup page identifies the June template and reports that the ADC/UI state
is being primed. After audio starts, the normal overview is shown unless a
parameter or state has just changed.

```text
JUNE MAIN|ALT
WAVE  TR:TRANSPOSE
A5:VELMODE  A6:KEYTRK
A7:LFOTGT   A8:GLIDE
P1:VALUE    P5:VALUE
P2:VALUE    P6:VALUE
P3:VALUE    P7:VALUE
P4:VALUE    P8:VALUE
```

The edit zoom appears for about `1.4 s` after a parameter or state change and
is redrawn at most every `50 ms`.

```text
EDIT MAIN|ALT
PARAMETER NAME
LARGE VALUE TEXT
SW1=Alt  SW2+K8=Level
```

## Build And Flash

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

Current software validation on 2026-06-06:

| Check | Result |
|---|---|
| `make` | PASS |
| Daisy QAE validator | PASS, `0 error(s), 0 warning(s)` |
| Hardware flash/run | Not performed in this pass |

Current build footprint:

| Region | Used | Capacity | Usage |
|---|---:|---:|---:|
| FLASH | `115688 B` | `128 KB` | `88.26%` |
| SRAM | `52920 B` | `512 KB` | `10.09%` |
| SDRAM | `0 B` | `64 MB` | `0.00%` |

## Maintenance Notes

Keep the synth intentionally small. This project is meant to be a reliable
Field control template, not a flagship instrument. Preserve the separation
between 1 ms control processing, capped visual refresh, and audio-only DSP
work. If additional banks or hidden pages are added, give each context its own
touch anchor so parameter values cannot jump when controls are reused.
