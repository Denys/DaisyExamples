# Control System Redesign Plan
## EDGE Performance FX — Page-Aware Pod Controls

**Status:** PLAN ONLY — not yet implemented
**Trigger:** User instruction to redesign so Pod K1/K2/SW2/Encoder change meaning per page, expanded by Shift

---

## 1. Problem with Current System

Every Pod control is hardwired to a single global parameter regardless of the active page:

| Control | Always does |
|---------|------------|
| Knob 1 | Delay Time |
| Knob 2 | Feedback |
| SW2 | Tap Tempo (global) |
| Pod Enc turn | Wet/Dry |
| Pod Enc push | Freeze Latch |
| Shift + K1 | Subdivision |
| Shift + K2 | FB LP macro |
| Shift + SW2 | Delay Mode toggle |
| Shift + Enc | Drive |
| Shift + Enc push | Page Jump |

Result: parameters on P2 (Tone), P3 (Motion), P4 (Freeze) can only be edited via the Ext Encoder menu — slow for live performance.

---

## 2. Design Goal

**Pod controls (K1, K2, SW2, Pod Enc) adapt to the current page.**
The most important parameters of each page are directly accessible without menu navigation.
Shift adds a second layer on the same page.

**Always-global (never change):**
- SW1 short tap → Freeze momentary
- SW1 hold → Shift modifier
- Ext Encoder + Ext PSH → menu navigation / precision editing (all pages)
- BAK → return to P1
- Shift + Pod Enc push → Page Jump (all pages)

---

## 3. Full Page-Aware Control Map

### P1 — Perform (unchanged from current)

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | Delay Time (subdiv snap or free ms) | Subdivision select (always snaps) |
| Knob 2 | Feedback 0–98% | FB LP cutoff macro (500–18000 Hz) |
| SW2 | **Tap Tempo** | Delay Mode toggle (Sync ↔ Free) |
| Pod Enc turn | Wet/Dry mix | Drive amount |
| Pod Enc push | Freeze Latch toggle | Page Jump |

---

### P2 — Tone

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | Input HP cutoff (20–500 Hz) | Feedback HP cutoff (20–500 Hz) |
| Knob 2 | Feedback LP cutoff (500–18000 Hz) | Wet Tone / Damping (1000–20000 Hz) |
| SW2 | **Toggle Bypass** (OFF ↔ ON) | Toggle Delay Mode |
| Pod Enc turn | Output Tilt (−6 to +6 dB, 0.5 dB/step) | Input Gain (0.5–2.0, 0.05/step) |
| Pod Enc push | Reset Output Tilt to 0 dB | Page Jump |

**Display hint bottom line:**
- Normal: `K1=HP  K2=FbLP  Enc=Tilt`
- Shift:  `K1=FbHP K2=Damp Enc=Gain`

---

### P3 — Motion

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | Wow Depth (0–100%) | Feedback (0–98%) — still useful |
| Knob 2 | Wow Rate (0.1–5.0 Hz) | Wet/Dry mix |
| SW2 | **Toggle Wow Enable** (ON ↔ OFF) | Tap Tempo |
| Pod Enc turn | Wow Rate fine (±0.1 Hz/step) | Drive amount |
| Pod Enc push | Toggle Wow Enable | Page Jump |

**Display hint:**
- Normal: `K1=Wow  K2=Rate  SW2=ON/OFF`
- Shift:  `K1=FB  K2=Mix  SW2=Tap`

---

### P4 — Freeze

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | Loop Size (50–2000 ms) | Delay Time (same as P1) |
| Knob 2 | Feedback (0–98%) | Wet/Dry mix |
| SW2 | **Toggle Hold Behavior** (Loop ↔ Hold-last) | Toggle Freeze Mode (Momentary ↔ Latch) |
| Pod Enc turn | Loop Size fine (±50 ms/step) | Diffuse Damping macro (1000–20000 Hz) |
| Pod Enc push | Freeze Latch toggle | Page Jump |

**Display hint:**
- Normal: `K1=Size  K2=FB  SW2=Mode`
- Shift:  `K1=Time  K2=Mix  SW2=LatchMode`

---

### P5 — Presets

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | Scroll preset cursor (maps 0–1 → 0–9) | — |
| Knob 2 | — (unused) | — |
| SW2 | **Load selected preset** (same as CON) | — |
| Pod Enc turn | Scroll preset cursor (±1/step, no pickup issue) | — |
| Pod Enc push | Load selected preset | Page Jump |

**Note on K1 pickup:** K1 uses jump mode for P5 scroll. Value snaps to the position of K1 on page entry. Users should be aware K1 position determines the starting cursor on P5.

---

### P6 — System

| Control | Normal | Shift + SW1 |
|---------|--------|-------------|
| Knob 1 | OLED Brightness (25/50/75/100%) | — |
| Knob 2 | — (unused) | — |
| SW2 | **Toggle Bypass** | Toggle Encoder Direction |
| Pod Enc turn | Cycle Brightness (25% steps) | — |
| Pod Enc push | Toggle Bypass | Page Jump |

---

## 4. Event System Changes Required

### New UIEvent values

```cpp
// Replace:
TAP,                  // → SW2_PRESS (page-aware)
DELAY_MODE_TOGGLE,    // → SW2_SHIFT (page-aware)

// Keep unchanged:
EXT_ENC_CW, EXT_ENC_CCW,
EXT_PSH_SHORT, EXT_PSH_LONG,
BAK, CON,
POD_ENC_PUSH,    // becomes page-aware inside Dispatch
PAGE_JUMP,       // Shift + Pod Enc push (global)
FREEZE_INSTANT,  // SW1 short (global)
```

### `main.cpp` SW2 dispatch changes

```cpp
// Before:
if (pod.button2.RisingEdge()) {
    if (shift_active) ui.Dispatch(UIEvent::DELAY_MODE_TOGGLE);
    else              ui.Dispatch(UIEvent::TAP);
}

// After:
if (pod.button2.RisingEdge()) {
    if (shift_active) { shift_consumed_ = true; ui.Dispatch(UIEvent::SW2_SHIFT); }
    else                ui.Dispatch(UIEvent::SW2_PRESS);
}
```

### Execution order in `main.cpp`

**Critical:** `PollControls()` must be called BEFORE button dispatches so that `shift_active_` is set inside UIState before `Dispatch(POD_ENC_PUSH)` etc. are called.

```
// Correct order:
1. pod.ProcessAllControls()
2. ext_enc.Debounce()
3. Compute shift_active (SW1 timing)
4. pod_enc_delta = pod.encoder.Increment()
5. ui.PollControls(k1, k2, pod_enc_delta, shift_active)   ← FIRST
6. Dispatch SW1 (freeze/shift events)
7. Dispatch SW2 (SW2_PRESS / SW2_SHIFT)
8. Dispatch Pod Enc push (POD_ENC_PUSH / PAGE_JUMP)
9. Dispatch Ext Enc events
10. ext_enc.SetFlipped()
11. Double-buffer sync
12. OLED draw
```

---

## 5. `PollControls()` Rewrite Structure

Replace current `PollKnobs()` with a page-aware `PollControls()`:

```cpp
void UIState::PollControls(float k1, float k2, int enc_delta, bool shift) {
    shift_active_ = shift;

    switch (page_) {
    case Page::PERFORM:
        // K1: delay time (subdiv or free)
        // K2: feedback
        // enc_delta: wet/dry (normal) or drive (shift)
        break;

    case Page::TONE:
        // K1: hp_hz (normal) or fb_hp_hz (shift)
        // K2: fb_lp_hz (normal) or diffuse_damping (shift)
        // enc_delta: output_tilt_db (normal) or input_gain (shift)
        break;

    case Page::MOTION:
        // K1: wow_depth (normal) or feedback (shift)
        // K2: wow_rate_hz (normal) or wet (shift)
        // enc_delta: wow_rate_hz fine (normal) or drive (shift)
        break;

    case Page::FREEZE:
        // K1: freeze_size_ms (normal) or delay time (shift)
        // K2: feedback (normal) or wet (shift)
        // enc_delta: freeze_size_ms fine (normal) or diffuse_damping (shift)
        break;

    case Page::PRESETS:
        // K1: scroll cursor (0-1 → 0-kNumPresets-1)
        // enc_delta: scroll cursor ±1
        break;

    case Page::SYSTEM:
        // K1: brightness control
        // enc_delta: brightness step
        break;
    }
}
```

---

## 6. `Dispatch()` Page-Aware Cases

### SW2_PRESS (unshifted)

```
P1 PERFORM : HandleTap()                 — Tap Tempo
P2 TONE    : params_->bypass = !bypass   — Toggle Bypass
P3 MOTION  : params_->wow_enabled ^= 1   — Toggle Wow
P4 FREEZE  : params_->freeze_loop_mode ^= 1  — Loop ↔ Hold
P5 PRESETS : LoadPreset(cursor_)         — Load
P6 SYSTEM  : params_->bypass ^= 1        — Toggle Bypass
```

### SW2_SHIFT (shifted)

```
P1 PERFORM : params_->sync_mode ^= 1    — Delay Mode toggle
P2 TONE    : params_->sync_mode ^= 1    — Delay Mode toggle
P3 MOTION  : HandleTap()                — Tap Tempo
P4 FREEZE  : params_->freeze_latch_mode ^= 1 — Mode toggle
P5 PRESETS : (unused)
P6 SYSTEM  : params_->enc_flipped ^= 1  — Enc Direction
```

### POD_ENC_PUSH (unshifted)

```
P1 PERFORM : freeze_latched ^= 1        — Freeze Latch
P2 TONE    : output_tilt_db = 0         — Reset Tilt
P3 MOTION  : wow_enabled ^= 1          — Toggle Wow
P4 FREEZE  : freeze_latched ^= 1        — Freeze Latch
P5 PRESETS : LoadPreset(cursor_)         — Load
P6 SYSTEM  : bypass ^= 1               — Toggle Bypass
```

---

## 7. Display Changes

Each page renderer should accept `shift_active` and show a context-aware bottom hint line:

```
P1: shift ? "K1=Sub K2=FbLP Enc=Drv" : "SW2=Tap Enc=Mix"
P2: shift ? "K1=FbHP K2=Damp Enc=Gain" : "K1=HP K2=FbLP Enc=Tilt"
P3: shift ? "K1=FB K2=Mix SW2=Tap" : "K1=Wow K2=Rate SW2=ON/OFF"
P4: shift ? "K1=Time K2=Mix Mode" : "K1=Size K2=FB SW2=Loop"
P5: "ENC/K1=SCROLL  CON=LOAD"
P6: "SW2=Bypass  Enc=Bright"
```

Function signatures change from:
```cpp
void DrawP2_Tone(int cursor, bool edit, const FxParams& p);
```
to:
```cpp
void DrawP2_Tone(int cursor, bool edit, bool shift, const FxParams& p);
```

---

## 8. Pickup Problem Note

When switching pages, pot K1 and K2 positions remain physical. Switching from P1 to P2 makes K1 jump the HP cutoff to its current physical position. This is **jump mode** — acceptable for Phase 1.

If pickup (catch) mode is later desired:
- Store `last_sent_k1[PAGE_COUNT]` and `last_sent_k2[PAGE_COUNT]`
- Only apply knob when `|knob_value - last_sent| < threshold` and then crosses stored value
- Implement as optional setting in P6

---

## 9. Implementation Checklist

When this plan is approved, implement in this order:

- [ ] Update `UIEvent` enum in `ui_state.h`
- [ ] Add `PollControls()` declaration in `ui_state.h` (replaces `PollKnobs()`)
- [ ] Rewrite `PollKnobs` → `PollControls` in `ui_state.cpp`
- [ ] Rewrite `Dispatch(SW2_PRESS)` per page in `ui_state.cpp`
- [ ] Rewrite `Dispatch(SW2_SHIFT)` per page in `ui_state.cpp`
- [ ] Rewrite `Dispatch(POD_ENC_PUSH)` per page in `ui_state.cpp`
- [ ] Update `main.cpp`: reorder PollControls before dispatches, update SW2 dispatch
- [ ] Update `display.h`: add `bool shift` param to P2-P6 renderers
- [ ] Update `display.cpp`: add context hints per page, shift-aware hints
- [ ] Build + verify zero errors
- [ ] Update `USER_MANUAL.md` control reference tables
- [ ] Update `Settings/SETTINGS.md` with new control map
