# Pod Tonestack + Nonlinear Filter Module Prototype
## Project Solution Proposal

---

### Document Information

| Field | Value |
|-------|-------|
| **Document Type** | Project Proposal / Design Document |
| **Version** | 1.0 |
| **Date** | 2026-03-31 |
| **Target Platform** | Daisy Pod + OLED + 2PB + Encoder |
| **Project Location** | `DaisyExamples/MyProjects/_projects/Pod_OledEncoder_test` |
| **Status** | **APPROVED - IN IMPLEMENTATION** |
| **Priority** | High |

---

## 1. Executive Summary

This proposal outlines the development of a **Daisy Pod filter module prototype** combining a **ToneStack** (3-band tone equalizer from DAFX library) with a **Nonlinear Filter** (NlFilt from DaisySP-LGPL) for guitar/bass tone shaping with analog-like overdrive characteristics.

The prototype extends the base hardware capabilities by implementing a **dual-mode control system** using SW1/SW2 hold-buttons, effectively tripling the number of available parameters from 3 to 9+ without additional hardware.

**Key Value Propositions:**
- Dual DSP chain (ToneStack → NlFilt) for comprehensive tone control
- Extended control surface via mode-switching (LFO + Envelope modes)
- Real-time OLED visualization of parameters and modulation
- Modular architecture for easy extension to other filter types
- Complete documentation for project handover

---

## 2. Project Description

### 2.1 Purpose Statement

Create a **standalone filter/effects module** for Daisy Pod that processes audio input through a ToneStack EQ followed by a Nonlinear Filter, providing guitarists and synth players with:
- Precise bass/mid/treble tonal control
- Analog-style overdrive and filter resonance effects
- LFO-based filter modulation (auto-wah style)
- Envelope-controlled filter sweeps
- Visual feedback via 128×64 OLED display

### 2.2 Target Users

| User Category | Use Case |
|--------------|----------|
| Guitarists | Direct tone shaping with overdrive |
| Bass Players | Low-end enhancement with harmonic overdrive |
| Synth Players | Filtered textural effects |
| Developers | Reference implementation for Pod+OLED projects |

### 2.3 Hardware Requirements

| Component | Specification | Notes |
|-----------|---------------|-------|
| **Daisy Pod** | Seed + Pod board | Main computing platform |
| **OLED Display** | SSD1306 128×64 via SPI | Visualization output |
| **2 Push-Buttons** | SW1, SW2 on Pod | Mode hold triggers |
| **Encoder** | Pod rotary encoder + click | Parameter selection |
| **2 Knobs** | Pod knob1, knob2 | Continuous parameters |

### 2.4 Software Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| **libDaisy** | Latest | Hardware abstraction |
| **DaisySP** | Latest | Core DSP modules |
| **DaisySP-LGPL** | Latest | NlFilt module (requires flag) |
| **DAFX_2_Daisy_lib** | Latest | ToneStack module |

### 2.5 Build Configuration

```makefile
# Makefile snippet - REQUIRED for LGPL modules
USE_DAISYSP_LGPL = 1

# Include paths
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP
DAFX_DIR = ../../MyProjects/DAFX_2_Daisy_lib
```

---

## 3. Design Decisions

### 3.1 DSP Architecture Choice

**Decision:** Serial chain: `ToneStack → NlFilt`

**Rationale:**
- ToneStack provides fundamental tone shaping before nonlinear processing
- NlFilt adds harmonic richness and overdrive characteristics
- Serial topology is simpler to implement and debug
- Allows independent bypass of each stage

**Alternative Considered:**
- Parallel blend (ToneStack × NlFilt) — rejected for complexity
- Reversed order (NlFilt → ToneStack) — rejected as EQ would smooth out NlFilt chaos

### 3.2 Control Mode Strategy

**Decision:** SW1/SW2 hold-buttons for mode switching

| Mode | Activated By | Knob1 Function | Knob2 Function | Encoder Function |
|------|-------------|----------------|----------------|-------------------|
| **Tone Mode** | Default (no SW held) | Bass (±1) | Treble (±1) | Middle (±1) |
| **LFO Mode** | SW1 held | LFO Rate (0.1–10 Hz) | LFO Depth (0–1) | LFO Wave (0–3) |
| **NL Mode** | SW2 held | Attack (0.01–2s) | Release (0.1–5s) | Coef "d" (0–3) |

**Rationale:**
- Pod has limited controls (2 knobs, 2 buttons, 1 encoder)
- Mode switching triples available parameters
- SW1/SW2 hold is intuitive (similar to shift keys)
- OLED provides clear mode indication

### 3.3 Filter Selection Rationale

| Module | Source | Selection Reason |
|--------|--------|-------------------|
| **ToneStack** | DAFX_2_Daisy_lib | Specifically designed for guitar tone stacking; well-documented in DAFX textbook |
| **NlFilt** | DaisySP-LGPL | Provides true analog-like nonlinearity; configurable coefficients allow range of overdrive tones |

**Why not other filters:**
- **Svf** (DaisySP) — good but less character than NlFilt
- **AutoWah** — covered by LFO mode can achieve similar
- **Phaser/Flanger** — can be added as future extension

### 3.4 Display Philosophy

**Decision:** Simple state-based display with parameter focus

- **Idle State:** Shows all three ToneStack levels as bar graph
- **Active State (knob move):** Highlights changed parameter with value
- **Mode Indication:** Clear banner when in LFO or NL mode

**Rationale:**
- Pod OLED is small (128×64)
- Focus on readability during performance
- Avoid complex menus — direct control is priority

### 3.5 Bypass Architecture

**Decision:** Per-stage bypass with LED indication

- Button1: ToneStack bypass (LED1 shows status)
- Button2: NlFilt bypass (LED2 shows status)
- Encoder click: Reset Middle to neutral (0)

**Rationale:**
- Independent bypass allows A/B comparison
- LED feedback essential for live performance
- Encoder click reset is convenient for returning to flat EQ

---

## 4. Detailed Controls Specification

### 4.1 Physical Control Mapping

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        POD CONTROL SURFACE                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│   ┌─────┐         ┌─────┐         ┌─────────┐        ┌─────┐        │
│   │KNOB1│         │KNOB2│         │ ENCODER │        │ BTN1│        │
│   │     │         │     │         │   (◉)   │        │     │        │
│   │  ○  │         │  ○  │         │  ╱ ╲    │        │  ○  │        │
│   │     │         │     │         │   ╲╱    │        │     │        │
│   └─────┘         └─────┘         └─────────┘        └─────┘        │
│   (Bass/         (Treble/        (Middle/          (Tone           │
│    LFO Rate)      LFO Depth)      Wave/Coef d)      Bypass)        │
│                                                                         │
│   ┌─────┐         ┌─────┐                     ┌─────┐                │
│   │ SW1 │         │ SW2 │                     │BTN2 │                │
│   │  ○  │         │  ○  │                     │  ○  │                │
│   │HOLD │         │HOLD │                     │     │                │
│   └─────┘         └─────┘                     └─────┘                │
│   (LFO Mode)     (NL Mode)                    (NlFilt              │
│                                                  Bypass)             │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### 4.2 Complete Parameter Table

#### Mode A: TONE Mode (Default)
| Control | Parameter | Min | Max | Default | Curve |
|---------|-----------|-----|-----|---------|-------|
| Knob1 | Bass | -1.0 | +1.0 | 0.0 | Linear |
| Knob2 | Treble | -1.0 | +1.0 | 0.0 | Linear |
| Encoder | Middle | -1.0 | +1.0 | 0.0 | Linear |
| Button1 | ToneStack Bypass | Off | On | Off | Toggle |
| Button2 | NlFilt Bypass | Off | On | Off | Toggle |
| Encoder Click | Reset | — | — | — | Momentary |

#### Mode B: LFO Mode (SW1 Held)
| Control | Parameter | Min | Max | Default | Curve |
|---------|-----------|-----|-----|---------|-------|
| Knob1 | LFO Rate | 0.1 Hz | 10 Hz | 1.0 | Log |
| Knob2 | LFO Depth | 0.0 | 1.0 | 0.5 | Linear |
| Encoder | LFO Wave | 0 (Sin) | 3 (Sqr) | 0 | Step |
| Button1 | ToneStack Bypass | Off | On | Off | Toggle |
| Button2 | NlFilt Bypass | Off | On | Off | Toggle |
| SW1 Hold | Enter LFO Mode | — | — | — | Hold |

#### Mode C: NL Mode (SW2 Held)
| Control | Parameter | Min | Max | Default | Curve |
|---------|-----------|-----|-----|---------|-------|
| Knob1 | Attack (Env) | 0.01s | 2.0s | 0.1 | Log |
| Knob2 | Release (Env) | 0.1s | 5.0s | 0.5 | Log |
| Encoder | NlFilt Coef "d" | 0.0 | 3.0 | 0.5 | Linear |
| Button1 | ToneStack Bypass | Off | On | Off | Toggle |
| Button2 | NlFilt Bypass | Off | On | Off | Toggle |
| SW2 Hold | Enter NL Mode | — | — | — | Hold |

### 4.3 Preset Definitions

#### Preset 1: Clean Guitar
```
Bass:     0.0
Mid:      0.0
Treble:   +0.2
NlFilt:  a=0, b=0, d=0, C=0, L=0 (bypassed effectively)
```

#### Preset 2: Overdrive
```
Bass:     +0.3
Mid:      +0.5
Treble:   -0.2
NlFilt:  a=1.5, b=-0.9, d=0.5, C=0, L=10
```

#### Preset 3: Fuzz Chaos
```
Bass:     +0.5
Mid:      +0.8
Treble:   0.0
NlFilt:  a=2.0, b=-1.5, d=1.0, C=0.1, L=20
```

---

## 5. OLED Display Menu Presentation

### 5.1 Screen Layout (128×64 pixels)

```
┌────────────────────────────────┐
│  ╔════════════════════════╗   │
│  ║   POD TONESTACK v1.0   ║   │
│  ╠════════════════════════╣   │
│  ║                        ║   │
│  ║ B: ████████░░  +0.3    ║   │
│  ║ M: ██████████   0.0    ║   │
│  ║ T: ██████░░░░  -0.2    ║   │
│  ║                        ║   │
│  ║ [NL] ON  [TS] ON       ║   │
│  ║                        ║   │
│  ╚════════════════════════╝   │
└────────────────────────────────┘
```

### 5.2 Mode Indicators

#### Default (Tone Mode)
```
╔════════════════════════╗
║      TONE MODE         ║
║ B: ████░░ M: ████░░    ║
║ T: ████░░              ║
╚════════════════════════╝
```

#### LFO Mode (SW1 Held)
```
╔════════════════════════╗
║      LFO MODE          ║
║ ╱╲ ╱╲ ╱╲ ╱╲ (wave)    ║
║ Rate: 2.5Hz  Depth:▓▓ ║
╚════════════════════════╝
```

#### NL Mode (SW2 Held)
```
╔════════════════════════╗
║      NL MODE          ║
║ Att: ████░░ Rel: ████░ ║
║ Coef d: ████░░ +0.5   ║
╚════════════════════════╝
```

### 5.3 Parameter Focus (Zoom View)

When any knob is moved, display shows focused parameter for 1.2 seconds:

```
╔════════════════════════╗
║      FOCUS: MID        ║
║                        ║
║        ████████░░      ║
║        +0.5            ║
║                        ║
╚════════════════════════╝
```

---

## 6. Technical Implementation Notes

### 6.1 Audio Callback Structure

```cpp
// Pseudo-code structure
void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                    AudioHandle::InterleavingOutputBuffer out,
                    size_t size) {
    for(size_t i = 0; i < size; i += 2) {
        float sig = in[i];  // L input (mono processing)
        
        // Process ToneStack (always active unless bypassed)
        if(!toneBypass) sig = toneStack.Process(sig);
        
        // Process NlFilt (always active unless bypassed)  
        if(!nlBypass) {
            nlFilt.ProcessBlock(&sig, &sig, 1);
        }
        
        // Output (mono -> stereo)
        out[i] = sig;
        out[i+1] = sig;
    }
}
```

### 6.2 LFO Implementation

The LFO mode modulates the NlFilt coefficients over time:
- Uses DaisySP `Oscillator` as LFO source
- LFO output (0 to 1) modulates coefficient "d" (nonlinear drive)
- Creates evolving, wah-like timbral changes

### 6.3 Envelope Implementation

The NL mode uses ADSR-style envelope to control coefficient "d":
- Attack: coefficient rises from 0 to target
- Release: coefficient falls from target to 0
- Creates filter sweep envelope (similar to auto-wah)

---

## 7. Project Schedule Estimate

| Phase | Description | Duration |
|-------|-------------|----------|
| **Phase 1** | Project setup, Makefile, basic audio chain | 1 day |
| **Phase 2** | ToneStack integration + controls | 1 day |
| **Phase 3** | NlFilt integration + bypass logic | 1 day |
| **Phase 4** | SW1/SW2 mode switching + LFO | 2 days |
| **Phase 5** | OLED visualization | 1 day |
| **Phase 6** | Testing, bug fixes, documentation | 2 days |
| **Total** | | **8 days** |

---

## 8. Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| NlFilt coefficient tuning is complex | Medium | Start with known presets; document coefficient relationships |
| OLED refresh may be slow | Low | Use frame limiting (60 FPS max) |
| Mode switching may cause audio glitches | Medium | Crossfade parameters during transition |
| LGPL licensing confusion | Low | Clear documentation of build flag requirement |

---

## 9. Future Extension Possibilities

| Extension | Description | Complexity |
|-----------|-------------|------------|
| Add **Phaser** post-NlFilt | Classic phaser effect | Medium |
| Add **Flanger** parallel to NlFilt | Temporal effects | Medium |
| MIDI control | CC-based parameter automation | Low |
| Preset storage (SD card) | Save/load user presets | Medium |
| Dual NlFilt | Cascade or parallel configuration | High |

---

## 10. Approval Formulair

### 10.1 Project Approval Request

This section is to be completed by the Project Lead for formal project kick-off approval.

---

#### Project Information

| Field | Entry |
|-------|-------|
| **Project Name** | Pod Tonestack + Nonlinear Filter Prototype |
| **Proposal Version** | 1.0 |
| **Proposal Date** | 2026-03-31 |
| **Requested By** | [Name] |
| **Target Completion** | [Date] |

#### Review Checklist

| # | Review Item | Status | Notes |
|---|-------------|--------|-------|
| 1 | Hardware requirements verified | ☐ Yes / ☐ No | |
| 2 | Software dependencies available | ☐ Yes / ☐ No | |
| 3 | DSP architecture approved | ☐ Yes / ☐ No | |
| 4 | Control scheme validated | ☐ Yes / ☐ No | |
| 5 | OLED design confirmed | ☐ Yes / ☐ No | |
| 6 | Schedule acceptable | ☐ Yes / ☐ No | |
| 7 | Risks identified and acceptable | ☐ Yes / ☐ No | |

#### Decision

| Option | Selection |
|--------|-----------|
| ☐ **APPROVED** | Project may proceed to implementation |
| ☐ **APPROVED WITH MODIFICATIONS** | See notes below |
| ☐ **REJECTED** | See reasons below |
| ☐ **DEFERRED** | Waiting for [conditions] |

#### Modifications / Rejection Reasons

```
[Write notes here if any modifications required or rejection reasons]
```

---

#### Signatures

| Role | Name | Signature | Date |
|------|------|-----------|------|
| **Proposal Author** | | | |
| **Technical Reviewer** | | | |
| **Project Lead** | | | |

---

#### Action Items Post-Approval

| # | Action Item | Owner | Due Date |
|---|-------------|-------|----------|
| 1 | Create project directory structure | | |
| 2 | Set up Makefile with LGPL flag | | |
| 3 | Implement basic audio chain | | |
| 4 | Integrate ToneStack module | | |
| 5 | Integrate NlFilt module | | |
| 6 | Implement control modes | | |
| 7 | Implement OLED display | | |
| 8 | Full system testing | | |
| 9 | Documentation completion | | |

---

## 11. Conclusion

This proposal presents a comprehensive plan for building a **Pod-based filter module prototype** using ToneStack and Nonlinear Filter. The design leverages all available Daisy Pod controls effectively through mode switching, provides clear visual feedback via OLED, and establishes a solid foundation for future extensions.

The estimated 8-day development timeline is realistic for a medium-complexity audio project, and the clear documentation ensures smooth handover to other developers.

**Recommendation:** Proceed with **APPROVAL** to begin implementation phase.

---

*Document prepared following daisy-qae workflow methodology*
*Last updated: 2026-03-31*