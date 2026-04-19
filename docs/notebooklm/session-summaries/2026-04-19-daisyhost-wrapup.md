# Session Summary — 2026-04-19

## What We Did
- Fixed the DaisyHost standalone startup layout regression by replacing the broad mute-banner hiding hack with a narrow `StandaloneUiPolicy`.
- Implemented DaisyHost Phase 1: canonical parameter contract, deterministic reset/state capture/restore, upgraded host session persistence, and parity-oriented tests.
- Implemented DaisyHost Phase 2: multi-app host support with `MultiDelay` as the default fixture and `Torus` as the first nontrivial hosted app.
- Fixed `MultiDelay` control spring-back and Torus idle-noise issues in the host wrappers.
- Added host-side debugging generators:
  - four independent `CV 1..4` generators
  - `Audio In 1` waveform generator for exciter/debug use
  - CV generator waveform amplitude with `0..2.5V` max plus DC bias so `2.5V bias + 2.5V amplitude` reaches the full `0..5V` span
- Tightened the DaisyHost Patch layout so the board fits the screen better and reworked the drawer so all four CV generators are visible at once.
- Added named Torus menu/display values for polyphony, models, and easter-egg FX, plus compact OLED-only aliases for long names.

## Decisions Made
- Keep DaisyHost as the board-faithful desktop host rather than replacing it with Eurorack- or Rack-first tooling.
- Treat `MultiDelay` as the deterministic regression fixture even after `Torus` was added as app #2.
- Use a host-side generator/debug layer for CV and audio input work instead of pushing those concepts into app cores.
- Allow plugin control semantics to diverge from the original `MultiDelayCore` control mapping as long as the underlying engine behavior and parameter values remain equivalent.
- Prioritize a canonical parameter contract and deterministic renderability before MetaController training or multi-node rack work.

## Key Learnings
- The stale standalone binary was the reason some Torus naming changes appeared not to land; the running `.exe` blocked relinking.
- The old standalone mute-banner workaround could scramble the entire JUCE layout because it hid sibling components and reclaimed the wrong bounds.
- For synth/debug work, CV and audio generators are more useful as generic host tools than as app-specific features.
- Short screen recordings are a good issue-capture format for this workspace because they reveal timing and interaction failures that screenshots miss.

## Open Threads
- DaisyBrain upload is blocked because local NotebookLM authentication has expired and needs `notebooklm login`.
- DAW-side manual VST3 validation remains a useful final check for the latest DaisyHost builds.
- Future roadmap items remain open: reusable app registry expansion, headless rendering/dataset generation, MetaController training gate, and possible multi-Daisy graphing later.

## Tools & Systems Touched
- `DaisyExamples/DaisyHost/`
- `patch/MultiDelay/`
- `patch/Torus/`
- CMake / CTest
- JUCE standalone + VST3 targets
- NotebookLM / DaisyBrain docs
