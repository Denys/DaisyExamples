# Noderr - Status Map

**Purpose:** This document tracks the development status of all implementable components (NodeIDs) defined in `noderr_architecture.md`. It guides task selection, groups related work via `WorkGroupID`, and provides a quick overview of project progress. It is updated by the AI Agent as per `noderr_loop.md`.

**Note:** All paths are relative to `DaisyExamples/noderr/` as root. The `specs/` directory is at `DaisyExamples/noderr/specs/`.

---

**Progress: 21% (5 / 24 VERIFIED)**

Breakdown: 5 Verified (libDaisy + DaisySP base + Reference Synths) | 0 WIP | 19 TODO | 24 Total

---

| Status | WorkGroupID | Node ID | Label | Dependencies | Logical Grouping | Spec Link | Classification | Notes / Issues |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 🟢 `[VERIFIED]` | | `LIBDASY_HWInit` | Hardware Initialization | — | libDaisy | [Spec](specs/LIBDASY_HWInit.md) | Critical | Existing, working |
| 🟢 `[VERIFIED]` | | `LIBDASY_AudioDriver` | Audio Callback Driver | `LIBDASY_HWInit` | libDaisy | [Spec](specs/LIBDASY_AudioDriver.md) | Critical | Existing, working |
| 🟢 `[VERIFIED]` | | `DSP_Oscillators` | Oscillators (Sine/Saw/Square/Tri) | — | DaisySP Core | [Spec](specs/DSP_Oscillators.md) | Standard | Existing, working |
| 🟢 `[VERIFIED]` | | `DSP_Filters` | Filters (LP/HP/BP/Notch) | `DSP_Oscillators` | DaisySP Core | [Spec](specs/DSP_Filters.md) | Standard | Existing, working |
| ⚪️ `[TODO]` | | `DSP_Delay` | Delay Effects (Echo/Ping-Pong) | `DSP_Filters` | DaisySP Core | [Spec](specs/DSP_Delay.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `DSP_Reverb` | Reverb Effects (Algorithmic) | `DSP_Delay` | DaisySP Core | [Spec](specs/DSP_Reverb.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `DSP_Envelope` | Envelope Generators (ADSR) | — | DaisySP Core | [Spec](specs/DSP_Envelope.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `DSP_Modulation` | Modulation Base (Chorus/Flanger/Phaser) | — | DaisySP Core | [Spec](specs/DSP_Modulation.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `FX_Chorus` | Chorus Effect | `DSP_Modulation` | Field Effects | [Spec](specs/FX_Chorus.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `FX_Flanger` | Flanger Effect | `DSP_Modulation` | Field Effects | [Spec](specs/FX_Flanger.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `FX_Phaser` | Phaser Effect | `DSP_Modulation` | Field Effects | [Spec](specs/FX_Phaser.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `FX_Sampler` | Sample Playback | `DSP_Filters` | Field Effects | [Spec](specs/FX_Sampler.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `FX_StringVoice` | String Synthesis (LGPL) | `DSP_Delay` | Field Effects | [Spec](specs/FX_StringVoice.md) | Standard | Existing; requires `USE_DAISYSP_LGPL=1` |
| ⚪️ `[TODO]` | | `FX_ModalVoice` | Modal Synthesis (LGPL) | `DSP_Envelope` | Field Effects | [Spec](specs/FX_ModalVoice.md) | Standard | Existing; requires `USE_DAISYSP_LGPL=1` |
| ⚪️ `[TODO]` | | `NIM_Granular` | Granular Processor | `DSP_Oscillators`, `DSP_Filters` | Nimbus | [Spec](specs/NIM_Granular.md) | Complex | Existing |
| ⚪️ `[TODO]` | | `NIM_SamplePlayer` | Sample Player | `NIM_Granular` | Nimbus | [Spec](specs/NIM_SamplePlayer.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `NIM_WSOLA` | WSOLA Time Stretching | `NIM_SamplePlayer` | Nimbus | [Spec](specs/NIM_WSOLA.md) | Complex | Existing |
| ⚪️ `[TODO]` | | `EX_Keyboard` | Field Keyboard Test Example | `LIBDASY_HWInit` | Examples | [Spec](specs/EX_Keyboard.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `EX_Midi` | MIDI Interface Example | `LIBDASY_HWInit` | Examples | [Spec](specs/EX_Midi.md) | Standard | Existing |
| ⚪️ `[TODO]` | | `DVPE_UI` | OLED / Hardware Runtime UI | — | DVPE Runtime | [Spec](specs/DVPE_UI.md) | Complex | **Missing** — firmware-side display & control rendering |
| ⚪️ `[TODO]` | | `DVPE_Compiler` | Generated C++ Module Integration | `DVPE_UI` | DVPE Runtime | [Spec](specs/DVPE_Compiler.md) | Complex | **Missing** — runtime loader for DVPE-generated code |
| ⚪️ `[TODO]` | | `DVPE_ParamControl` | Runtime Parameter Control | `DVPE_UI` | DVPE Runtime | [Spec](specs/DVPE_ParamControl.md) | Standard | **Missing** — knob/CV-to-parameter mapping at runtime |
| ⚪️ `[TODO]` | | `DVPE_PresetManager` | Preset Storage & Recall | `DVPE_UI` | DVPE Runtime | [Spec](specs/DVPE_PresetManager.md) | Standard | **Missing** — flash-based preset storage |
| 🟢 `[VERIFIED]` | | `SYNTH_Pollen8` | Pollen8 VA+FM 2.0 Reference Synth | `DSP_Oscillators`, `DSP_Filters`, `DSP_Envelope`, `DSP_Modulation`, `FX_Chorus` | Reference Synths | [Spec](specs/SYNTH_Pollen8.md) | Complex | Pre-compiled binary; closed source; 8-voice VA+FM polyphonic synth for Field |

---

### Legend for Status:

- ⚪️ **`[TODO]`**: Task is defined and ready to be picked up if dependencies are met. Also applies to `REFACTOR_` tasks created from technical debt.
- 📝 **`[NEEDS_SPEC]`**: Node identified in the architecture but requires a detailed specification to be drafted.
- 🟡 **`[WIP]`**: Work In Progress. The AI Agent is currently working on this node as part of the specified `WorkGroupID`.
- 🟢 **`[VERIFIED]`**: The primary completion state. Node implemented, all ARC Verification Criteria met, spec finalized to "as-built", outcomes logged.
- ❗ **`[ISSUE]`**: Significant issue or blocker identified. Details in `noderr_log.md` or linked in Notes column.

---

### Notes on Columns:

- **Status**: The current state of the NodeID (see Legend above).
- **WorkGroupID**: Unique ID assigned to a "Change Set" of nodes worked on together. Blank unless `[WIP]`.
- **Node ID**: Unique identifier for the component, matching `noderr_architecture.md`.
- **Label**: Concise, human-readable name for the NodeID.
- **Dependencies**: NodeIDs that must be `[VERIFIED]` before work on this node can begin.
- **Logical Grouping**: Tag categorizing nodes by feature/module/layer.
- **Spec Link**: Relative Markdown link to the corresponding spec file in `specs/`.
- **Classification**: `Critical` (system stability), `Complex` (significant logic, multiple deps), or `Standard` (clear I/O, minimal logic).
- **Notes / Issues**: Brief comments, LGPL flags, or blockers.

---

### Progress Breakdown by Group:

| Group | Verified | WIP | TODO | Total |
| :--- | :--- | :--- | :--- | :--- |
| libDaisy | 2 | 0 | 0 | 2 |
| DaisySP Core | 2 | 0 | 4 | 6 |
| Field Effects | 0 | 0 | 6 | 6 |
| Nimbus | 0 | 0 | 3 | 3 |
| Examples | 0 | 0 | 2 | 2 |
| DVPE Runtime | 0 | 0 | 4 | 4 |
| Reference Synths | 1 | 0 | 0 | 1 |
| **TOTAL** | **5** | **0** | **19** | **24** |

---

*(This table is maintained by the AI Agent as per `noderr_loop.md`. Update Status and WorkGroupID columns as Change Sets are processed. Mark VERIFIED only when ARC verification is fully passed.)*
