# EDGE Performance FX — Parameter Tables

Per-page tables listing every effect, every variable, its `FxParams` field, value range,
and the physical control assigned to it.

**Control notation:**
- **Bold control** = live hardware control, works anytime from that page
- `Ext Enc (edit)` = Ext Enc navigates cursor → Ext PSH enters edit mode → Ext Enc turns value → Ext PSH / CON confirms
- `—` = no assigned control (fixed, internal, or read-only)

---

## P1 — Perform

> Performance layer. All controls directly accessible — no menu navigation required.

| Effect / Module | Variable | `FxParams` Field | Range | Assigned Control |
|---|---|---|---|---|
| **Delay Core** | Delay Time (sync) | `delay_time_beats` | 1/16 · 1/8 · 1/4 · 1/2 · 1/1 · dotted · triplet | **Knob 1** |
| **Delay Core** | Delay Time (free) | `delay_time_ms` | 10 – 2 000 ms | **Knob 1** (when `sync_mode = false`) |
| **Delay Core** | Subdivision step | *(derived from K1 zone snap)* | 10 quantised divisions | **Shift + Knob 1** |
| **Delay Core** | Delay Mode | `sync_mode` | Sync / Free | **Shift + SW2** |
| **Delay Core** | BPM | `bpm` | 40 – 240 BPM | **SW2** (tap) · `Ext Enc (edit)` |
| **Delay Core** | Feedback | `feedback` | 0 – 98 % (hard clamped) | **Knob 2** |
| **Wet/Dry Mix** | Mix | `wet` | 0 – 100 % | **Pod Enc** (turn) |
| **Drive** | Drive Amount | `drive` | 0 – 100 % | **Shift + Pod Enc** (turn) |
| **Freeze Engine** | Freeze (momentary) | *(volatile flag)* | ON while held | **SW1** (short tap — hold to sustain) |
| **Freeze Engine** | Freeze Latch | `freeze_latched` | Latched / Unlatched | **Pod Enc** (push) |
| **Freeze Engine** | Freeze Mode | *(config — see P4)* | Momentary / Latch | **Shift + Ext PSH** (long hold) |

---

## P2 — Tone

> Tonal shaping of every filter stage. All parameters edited via Ext Enc menu.
> Exception: Feedback LP also has a direct macro on Shift + Knob 2.

| Effect / Module | Variable | `FxParams` Field | Range | Assigned Control |
|---|---|---|---|---|
| **Input HP Filter** *(Svf)* | Cutoff Frequency | `hp_hz` | 20 – 500 Hz | `Ext Enc (edit)` |
| **Input HP Filter** *(Svf)* | Resonance | — | 0.0 fixed | — internal |
| **Feedback LP Filter** *(Svf)* | Cutoff Frequency | `fb_lp_hz` | 500 – 18 000 Hz | `Ext Enc (edit)` · **Shift + Knob 2** (macro) |
| **Feedback LP Filter** *(Svf)* | Resonance | — | 0.0 fixed | — internal |
| **Feedback HP Filter** *(Svf)* | Cutoff Frequency | `fb_hp_hz` | 20 – 500 Hz | `Ext Enc (edit)` |
| **Feedback HP Filter** *(Svf)* | Resonance | — | 0.0 fixed | — internal |
| **Dark Diffusion** *(ReverbSc)* | Damping LP Freq | `diffuse_amt` | 1 000 – 20 000 Hz | `Ext Enc (edit)` |
| **Dark Diffusion** *(ReverbSc)* | Room Feedback | — | 0.85 fixed | — internal |
| **Output Tilt** *(Biquad)* | Tilt Amount | `output_tilt_db` | −6 to +6 dB | `Ext Enc (edit)` |
| **Output Tilt** *(Biquad)* | Tilt Pivot Frequency | — | 1 000 Hz fixed | — internal |

---

## P3 — Motion

> Modulation controls for the wow/flutter delay-tap modulator.
> All parameters edited via Ext Enc menu.

| Effect / Module | Variable | `FxParams` Field | Range | Assigned Control |
|---|---|---|---|---|
| **Wow / Flutter** *(Phasor)* | Depth | `wow_depth` | 0 – 100 % | `Ext Enc (edit)` |
| **Wow / Flutter** *(Phasor)* | Rate | `wow_rate_hz` | 0.10 – 5.00 Hz | `Ext Enc (edit)` |
| **Wow / Flutter** *(Phasor)* | Enable | `wow_enabled` | ON / OFF | `Ext Enc (edit)` · **Ext PSH** (toggle) |

---

## P4 — Freeze

> Detailed freeze engine configuration. All parameters edited via Ext Enc menu.
> Fast freeze controls (SW1, Pod Enc push) are always live from any page.

| Effect / Module | Variable | `FxParams` Field | Range | Assigned Control |
|---|---|---|---|---|
| **Freeze Engine** | Freeze Mode | `freeze_latched` *(mode config)* | Momentary / Latch | `Ext Enc (edit)` |
| **Freeze Engine** | Hold Behavior | *(internal state)* | Loop / Hold-last | `Ext Enc (edit)` |
| **Freeze Engine** | Loop Size | `freeze_size_ms` | 50 – 2 000 ms | `Ext Enc (edit)` |
| **Freeze Engine** | Reverse Playback | `freeze_reverse` | ON / OFF | — future release |
| **Freeze Engine** | Engage / Release crossfade | *(internal)* | 5 – 10 ms fixed | — internal (click prevention) |

---

## P5 — Presets

> Preset load, save, and init. All actions via Ext Enc + CON/PSH.
> Quick save (Ext PSH long) available from any page without entering P5.

| Component | Action | Stored Fields | Slots / Range | Assigned Control |
|---|---|---|---|---|
| **Preset Store** | Load Preset | all `FxParams` fields | Slots 1 – 8 | **Ext Enc** (navigate) + **CON** (confirm) |
| **Preset Store** | Save Preset | all `FxParams` fields | Slots 1 – 8 | **Ext Enc** (navigate) + **CON** · **Ext PSH long** (quick save, any page) |
| **Preset Store** | Init / Default | all `FxParams` fields | factory values | **Ext Enc** (navigate) + **CON** |
| **Preset Store** | Preset Name | — | read-only label | — |
| **Preset Store** | Slot validity flag | — | valid / corrupt | — auto-checked on load |

---

## P6 — System

> Hardware configuration and firmware info. All editable items via Ext Enc menu.

| Component | Variable | Field / Driver | Range | Assigned Control |
|---|---|---|---|---|
| **Display** | OLED Brightness | *(hw contrast reg)* | 25 / 50 / 75 / 100 % | `Ext Enc (edit)` |
| **Ext Board** | Encoder Direction | `ext_enc.SetFlipped()` | Normal / Flipped | `Ext Enc (edit)` |
| **Audio Engine** | Bypass Mode | `bypass` | Active / Bypass | `Ext Enc (edit)` |
| **Audio Engine** | Sample Rate | — | 48 kHz | — read-only |
| **Audio Engine** | Block Size | — | 48 samples | — read-only |
| **Firmware** | Version | — | vX.Y.Z | — read-only |
