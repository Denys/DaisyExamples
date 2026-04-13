# Phase 2 Audio Skeleton Design

**Date:** 2026-04-12
**Project:** `POD_EDGE_mono_DSP`

## Scope

Implement the narrow Phase 2 audio skeleton only:

- input gain
- `DcBlock`
- input `Svf` high-pass
- `Overdrive`
- simple ISR-safe smoothing for continuous audio parameters

Do not implement yet:

- SDRAM delay
- feedback filters in active audio path
- reverb
- limiter
- freeze audio engine
- page-aware control redesign

## Architecture

Phase 2 stays inside `main.cpp` so the current UI, display, preset, and control system remain unchanged. The audio callback will continue reading only the active parameter buffer and will remain real-time safe.

DSP module initialization will be enabled in `main()`, and the callback will process a short chain:

`input_gain -> DcBlock -> Svf HP -> Overdrive -> dual-mono out`

Parameter smoothing will be done in the callback with lightweight stateful smoothing variables for:

- `input_gain`
- `drive`
- `hp_hz`

The feedback, wet, wow, freeze, and delay-related parameters will remain available in the parameter store but inactive in the audio path until later phases.

## Risk Controls

- Keep all hardware/UI work in the main loop only.
- Keep all DSP state static and preallocated.
- Avoid delay/reverb/freeze changes in this phase to preserve build and runtime stability.
- Clamp audio parameters before use where instability is possible.

## Verification

Primary verification for this phase is a clean rebuild plus a code-level review that the callback remains ISR-safe and the build memory footprint stays reasonable.
