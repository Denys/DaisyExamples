# LATEST_PROJECTS.md

Volatile companion to `AGENTS.md`.

Refresh this file when new custom firmware projects become active or when a
recent project's verification/documentation status changes materially.

## Last Refreshed

- 2026-04-18

## How To Use This File

1. Find the target project here if it is part of the recent working set.
2. Use the listed local docs before relying on older repo-wide patterns.
3. Treat dated plan docs as design context, not implementation truth.
4. If the target project is not listed here, fall back to `AGENTS.md` plus the
   nearest `README.md`, `CHECKPOINT.md`, and local source tree.

## Refresh Policy

- Always consider pinned workspace roots first: `DaisyDAFX/`, `DaisyHost/`,
  `pedal/`, `DAISY_QAE/`, and `MyProjects/_projects/`.
- Keep the pinned workspace roots in sync with `AGENTS.md`; if that list
  changes, update both files in the same change.
- Do not infer "recent" from one subtree only.
- Rank recency from file-level changes, not directory mtimes.
- Ignore generated trees such as `build/`, `dist/`, `.worktrees/`, `.tmp/`, and
  `node_modules/` when scoring activity.
- Use `py -3 ./ci/list_recent_project_roots.py` to refresh the candidate list
  before editing this file.
- If this file is stale relative to the current task or the target is not
  listed here, treat it as a hint and fall back to `AGENTS.md`, the nearest
  local docs, and `py -3 ./ci/list_recent_project_roots.py`.

## Recent Custom Project Entries

| Project | Board | Latest Activity | Key Local Docs | Agent Notes |
|---------|-------|-----------------|----------------|-------------|
| `field/CloudSeed/` | Field | 2026-04-18 | `README.md` | New Field-first CloudSeed example using the real imported `third_party/CloudSeedCore` engine instead of the earlier Seed fallback bridge. Build-verified with the controller object placed in SDRAM; current memory headroom is tight on SDRAM but links cleanly. |
| `field/GranularSynth/` | Field | 2026-04-18 | `README.md` | New Field granular-style synth voice built on `GrainletOscillator`. Build-verified after enabling the DaisySP LGPL path required by its `ReverbSc` layer. |
| `DaisyHost/` | Host-side Daisy Patch workspace | 2026-04-18 | `README.md`, `AGENTS.md`, `CHECKPOINT.md`, `CHANGELOG.md` | First-party JUCE/CMake workspace for the DaisyHost Patch plugin and standalone app. Current source version is `0.2.0`; host validation passed with `23/23` tests, both `DaisyHost Patch.vst3` and `DaisyHost Patch.exe` rebuilt successfully, and `patch/MultiDelay` clean-rebuilt against the shared `MultiDelayCore`. Remaining manual gap is a DAW-side VST3 load pass; use the local docs before editing because this workspace is intended for parallel agent work. |
| `pedal/` | DaisyPedal workspace | 2026-04-18 | `README.md`, child project `README.md` files under `PassthruBypass/`, `NoiseGate/`, `PitchDrop/`, `PolyOctave/` | Fundamental first-party workspace. Phase-1 native integration now includes a `DaisyPedal` board helper in `libDaisy`, a `DaisyPedal` sublibrary in `DaisySP`, helper scaffolding via `py -3 helper.py create --board pedal`, build-verified `pedal/` examples, and a checked-in Doxygen pipeline targeting `docs/daisypedal_reference.pdf`. |
| `DaisyDAFX/` | Library workspace | 2026-04-18 | `README.md`, `CHECKPOINT.md`, `docs/verification_matrix.md`, `docs/candidate_review.md`, `docs/daisydafx_reference.pdf`, `AGENTS.md` | Fundamental first-party workspace. Canonical DAFX library root beside `DaisySP` and `libDaisy`; prefer it over `MyProjects/DAFX_2_Daisy_lib`. Latest completed pass canonicalized naming, corrected `WahWah` manual-wah semantics, strengthened host-side regressions to `244/244` passing tests, and generated `docs/daisydafx_reference.pdf`. The latest verified state currently lives in the isolated worktree branch `codex/daisydafx-canonicalization` under `C:\\Users\\denko\\.codex\\worktrees\\dafxcanon` until merged back into the main checkout. Ignore `build/` when judging recency. |
| `MyProjects/_projects/Seed_CloudSeedCoreBridge/` | Seed | 2026-04-18 | `README.md`, `CLOUDSEED_BLOCK_DIAGRAM.md`, `CLOUDSEED_BLOCK_DIAGRAM.html`, `CLOUDSEED_PARAMETER_BREAKDOWN_BKS.md`, `CLOUDSEED_SOURCE_TAXONOMY_BKS.md` | Bridge/scaffolding state for future CloudSeedCore integration. External CloudSeedCore import is still pending. No local `CHECKPOINT.md` was found. The README currently contains unresolved merge markers near the diagram-reference section, so cross-check the companion docs directly. |
| `MyProjects/_projects/Pod_MultiFX_Chain/` | Pod | 2026-04-15 | `README.md`, `tests/pod_multifx_pages_test.cpp` | Most fully verified recent Pod project. README records passing host-side test, QAE lint, and clean firmware rebuild on 2026-04-15. |
| `MyProjects/_projects/multi_fx_synth_pod/` | Pod | 2026-04-14 | `CONTROLS.md`, `README.md` | `CONTROLS.md` is newer than `README.md`; prefer controls doc and source over README when behavior details conflict. Uses DaisySP LGPL compressor. |
| `MyProjects/_projects/Pod_Harmonizer/` | Pod | 2026-04-14 | `README.md`, `CONTROLS.md` | Intermediate harmonizer effect with dual pitch-shift voices and preset cycling. No local `CHECKPOINT.md` was found. |
| `MyProjects/_projects/Seed_MIDI_Grainlet/` | Seed | 2026-04-01 | `README.md`, `TEENSY_4_SETUP_AND_PROGRAM.md` | Seed plus Teensy companion workflow. Use local Teensy setup doc before making controller-side assumptions. |

## Recent Plan Docs Worth Checking

- `docs/plans/2026-04-15-daisy-qae-reasonable-controls.md`
- `docs/plans/2026-04-15-daisy-qae-reasonable-controls-design.md`
- `docs/plans/2026-04-15-pod-multifx-reattach-slew.md`
- `docs/plans/2026-04-15-pod-multifx-reattach-slew-design.md`
- `docs/plans/2026-04-14-pod-multifx-lfo-page.md`
- `docs/plans/2026-04-14-pod-multifx-page-controls.md`
- `docs/plans/2026-03-29-seed-midi-grainlet.md`
- `docs/plans/2026-03-29-seed-midi-grainlet-teensy.md`

## Notes

- `ci/build_examples.py` excludes `MyProjects/`, so recent custom projects need
  targeted validation from their own directories.
- `DaisyDAFX/` now has its own host-side validation/doc workflow; do not route
  it through the default Make-based firmware assumptions used for board targets.
- The active custom-project working set now includes new first-party Field work
  alongside the recent Pod, Seed, and Pedal efforts.
- If a new project becomes the main focus, add it here instead of expanding
  `AGENTS.md`.
