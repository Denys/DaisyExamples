# Noderr Log: Field_AmbientGarden Firmware

_Entries are prepended (newest first)._

---

**Type:** ProjectInit + Audit
**Timestamp:** 2026-03-06T00:00:00Z
**WorkGroupID:** init-20260306-000000
**NodeID(s):** ALL (22 nodes — initial retrofit scan)
**Logged By:** AI-Agent (Claude Sonnet 4.6)
**Details:**
Initial Noderr specialization applied to existing Field_AmbientGarden firmware project.
Code audit performed against Daisy Field development standards (daisy_cpp SKILL v1.0).

**Audit Summary:**

✅ CORRECT IMPLEMENTATIONS:
- Non-interleaved audio buffers: `out[0][i]`, `out[1][i]`
- `hw.StartAdc()` called before `hw.StartAudio()`
- `hw.ProcessAllControls()` in main loop only (not in AudioCallback)
- OLED updates in main loop only
- `Params` struct for safe main→callback communication
- A/B keys used for app control (not melody) — correct Field usage
- `System::Delay(16)` for ~60Hz main loop rate
- Reverb and LPF parameters smoothed correctly in AudioCallback

🔴 BUG-1 — CRITICAL: `rand()` in AudioCallback
- `RandomFloat()` at line 149 calls `rand()` which is NOT real-time safe
- `rand()` acquires a mutex internally; can cause audio dropouts
- Called from `OnClockTrigger()` which is invoked per-sample in AudioCallback
- Fix: Replace with xorshift32 (lock-free, callback-safe)
- WorkGroupID: fix-20260306-BUG1 → NodeID: FW_ClockTrigger

🟡 BUG-2 — SIGNIFICANT: Dead smoothing code
- `smoothed.brightness/damping/structure` computed per-block in AudioCallback (lines 282-286)
- These values are NEVER applied to modal[v] — raw params used in ProcessKnobs() instead
- Dead code; potential for confusion in future maintenance
- Fix: Remove the 3 dead smoothing lines; rely on 60Hz ProcessKnobs() rate
- WorkGroupID: fix-20260306-BUG2 → NodeID: FW_AudioCallback

⚠️ NON-CONFORMANCES (low priority):
- NC-1: FieldUX not used (field_defaults.h helpers used instead) — see REFACTOR_FieldUX
- NC-2: Custom `Smooth()` instead of `fonepole()` — cosmetic, see REFACTOR_Smooth
- MISMATCH-1: Frozen OLED state documented in CONTROLS.md but not implemented — see REFACTOR_FrozenOLED

**Architecture Learnings:**
- GEN_RandomClock drives the generative clock per-sample; 60-70% CPU budget used by 4x ModalVoice
- FW_ParamBridge (Params struct) correctly separates main-loop writes from callback reads
- Stereo width implemented via per-voice panning (V0,V2 = left-biased; V1,V3 = right-biased)

**Specification Initialization:**
- noderr_project.md: Written (project constitution)
- noderr_architecture.md: Written (22-node Mermaid diagram)
- noderr_tracker.md: Written (22 nodes TODO + 2 WIP bug fixes)
- specs/: Priority 5 specs created for FW_AudioCallback, FW_ParamBridge, FW_ClockTrigger, DSP_ModalVoices, GEN_TuringMachine

---
