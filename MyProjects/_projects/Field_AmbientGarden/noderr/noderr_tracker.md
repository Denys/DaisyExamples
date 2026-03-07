# Noderr Tracker: Field_AmbientGarden Firmware

**Last Updated:** 2026-03-06
**Total Nodes:** 22
**Active WorkGroupIDs:** fix-20260306-BUG1, fix-20260306-BUG2

---

## Node Status Dashboard

| Status | NodeID | Description | Dependencies | Classification | WorkGroupID |
|--------|--------|-------------|--------------|----------------|-------------|
| 🟡 WIP | `FW_ClockTrigger` | OnClockTrigger() voice distribution | GEN_*, DSP_ModalVoices, FW_ParamBridge | Critical | fix-20260306-BUG1 |
| 🟡 WIP | `FW_AudioCallback` | Audio processing entrypoint | All DSP_, FW_ParamBridge, GEN_RandomClock | Critical | fix-20260306-BUG2 |
| ⚪ TODO | `HW_Platform` | DaisyField hw: Init, StartAdc, StartAudio | — | Standard | |
| ⚪ TODO | `HW_Knobs` | 8× hw.knob[0-7].Process() | HW_Platform | Standard | |
| ⚪ TODO | `HW_KeysA` | A1–A8 scale selection (exclusive) | HW_Platform | Standard | |
| ⚪ TODO | `HW_KeysB` | B1–B8 preset selection (exclusive) | HW_Platform | Standard | |
| ⚪ TODO | `HW_Switches` | SW1 freeze / SW2 stereo width | HW_Platform | Standard | |
| ⚪ TODO | `HW_OLED` | 128×64 display via hw.display | HW_Platform | Standard | |
| ⚪ TODO | `HW_LEDs` | led_driver + FieldKeyboardLEDs | HW_Platform | Standard | |
| ⚪ TODO | `GEN_TuringMachine` | 8-bit shift register PRNG (turing_machine.h) | — | Complex | |
| ⚪ TODO | `GEN_ScaleQuantizer` | Scale + root quantization (scale_quantizer.h) | CFG_ScaleSystem | Complex | |
| ⚪ TODO | `GEN_RandomClock` | Probabilistic trigger generator (random_clock.h) | HW_Platform (sr) | Standard | |
| ⚪ TODO | `DSP_ModalVoices` | 4× ModalVoice LGPL — physical synthesis | HW_Platform (sr) | Complex | |
| ⚪ TODO | `DSP_PreLPF` | Stereo SVF lowpass pre-reverb | HW_Platform (sr) | Standard | |
| ⚪ TODO | `DSP_Reverb` | ReverbSc LGPL — stereo reverb | HW_Platform (sr) | Standard | |
| ⚪ TODO | `DSP_SoftClip` | tanhf() output saturation | — | Standard | |
| ⚪ TODO | `FW_ParamBridge` | Params + SmoothedParams structs | All HW_Knobs, CFG_* | Standard | |
| ⚪ TODO | `FW_KnobProcessor` | ProcessKnobs() mapping + param targets | HW_Knobs, FW_ParamBridge | Standard | |
| ⚪ TODO | `UI_OLEDRenderer` | Display state machine (title/voice/param/status) | FW_ParamBridge, FW_ClockTrigger, CFG_* | Complex | |
| ⚪ TODO | `UI_LEDAnimator` | Knob + A/B rows + switch LEDs | FW_ParamBridge, FW_ClockTrigger, CFG_* | Standard | |
| ⚪ TODO | `CFG_VoicePresets` | 8 preset definitions (Glass → Temple) | — | Standard | |
| ⚪ TODO | `CFG_ScaleSystem` | 8 scale tables + root semitone mapping | — | Standard | |

---

## Active Bug Fix Work Items

| WorkGroupID | Type | Goal | Nodes Affected | Status |
|-------------|------|------|----------------|--------|
| `fix-20260306-BUG1` | fix | Replace `rand()` with xorshift32 PRNG in AudioCallback path | `FW_ClockTrigger` | 🟡 WIP |
| `fix-20260306-BUG2` | fix | Remove dead smoothing code for brightness/damping/structure in AudioCallback | `FW_AudioCallback` | 🟡 WIP |

---

## Technical Debt Backlog

| ID | Description | Source NodeID | Priority |
|----|-------------|---------------|----------|
| `REFACTOR_FieldUX` | Migrate from field_defaults.h helpers to FieldUX library | `UI_OLEDRenderer`, `UI_LEDAnimator` | Low |
| `REFACTOR_FrozenOLED` | Implement "FROZEN" OLED state per CONTROLS.md spec | `UI_OLEDRenderer` | Low |
| `REFACTOR_Smooth` | Replace custom `Smooth()` with DaisySP `fonepole()` | `FW_AudioCallback` | Cosmetic |

---

## Status Legend

| Icon | Status | Meaning |
|------|--------|---------|
| 🟢 | VERIFIED | Implemented, spec written, ARC criteria met |
| 🟡 | WIP | In active development under a WorkGroupID |
| ⚪ | TODO | Not yet specced or implemented (retrofit pending) |
| 🔴 | BLOCKED | Cannot proceed — dependency or hardware issue |
