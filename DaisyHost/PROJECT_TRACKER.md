# DaisyHost Project Tracker

Last updated: 2026-04-22

Use this file as the running DaisyHost status ledger. Update it after each
meaningful implementation or verification iteration so the active work order,
testing evidence, parallel-thread coordination, and next steps stay explicit.

Verification basis for this iteration: source/docs inspection plus a fresh
wrapper-driven host gate rerun and fresh firmware reference builds from this
checkout. `cmake`, `ctest`, and `make` are on `PATH`, `DaisyHost/build` is
present, `.\build_host.cmd` reran the full DaisyHost host gate successfully,
and `ctest` passed `79/79`, including the W4/W5 helper coverage, the new
CloudSeed core/app/render coverage, plus `DaisyHostStandaloneSmoke` and
`DaisyHostRenderSmoke`. `make` also passed from `patch/MultiDelay/` and
`patch/Torus/`, with the Torus clean rebuild using a PowerShell `build/`
removal because `make clean` is not Windows-safe in this checkout. The
remaining intentional runtime gap is external DAW/VST3 load validation, now
deferred until post-WS7.

Historical note: earlier DaisyHost docs drifted on host test counts and related
verification assertions. Those stale numeric claims are now removed or
relabeled until the runtime commands are rerun in a future iteration.

## Mandatory Iteration Testing Notice

Every implementation iteration must record:

- date
- thread or agent
- affected slice or files
- exact tests, builds, or checks run
- result
- blockers and handoff

No iteration is complete without test evidence.

For code changes, follow test-first discipline:

1. write the failing test first
2. verify the failure is for the expected reason
3. implement the minimal change
4. rerun the targeted tests
5. update the tracker and any truth-changed docs

For docs-only iterations, run doc consistency checks and, when the roadmap
artifact changes, also run:

```sh
d2 validate DaisyHost/ROADMAP.d2
d2 DaisyHost/ROADMAP.d2 DaisyHost/ROADMAP.svg
```

## High-Priority Tracking Files

| File | Role | Review / update rule |
|---|---|---|
| `AGENTS.md` | Ownership slices, shared contracts, and handoff rules. | Review every implementation iteration; update when ownership, testing policy, or concurrency rules change. |
| `PROJECT_TRACKER.md` | Active roadmap, recommended work order, per-iteration testing ledger, and handoffs. | Review every implementation iteration and always update it before closeout. |
| `CHECKPOINT.md` | Current source-backed state plus last recorded runtime verification. | Review every implementation iteration; update when verification truth, artifact paths, or recorded gates change. |
| `CHANGELOG.md` | Durable user-facing and workflow policy history. | Review every implementation iteration; update only when a durable policy or release-history fact changed. |

Cadence:

- review all high-priority tracking files every implementation iteration
- always update `PROJECT_TRACKER.md`
- update the other high-priority tracking files when their truth changed

Current implementation status:

- Workstream 4 is now landed: DaisyHost exposes a stable five-slot DAW
  automation bank with fixed ids `daisyhost.slot1` through `daisyhost.slot5`.
- Workstream 5 is now landed: the processor exposes an in-process
  effective-state snapshot/readback API for live parameters, CV, gate, and
  audio-input state.
- Workstream 6 is now active and the first hosted-app pilot is landed:
  `cloudseed` wraps a new portable `DaisyCloudSeedCore`, adds page-based knob
  remapping (`Space` / `Motion`), and ships a checked-in render smoke scenario.
- The local checkout now has a standard host-build wrapper:
  `build_host.cmd` -> `build_host.ps1`.
- External DAW/VST3 load validation is intentionally deferred until a
  post-Workstream-7 manual pass.
- Next planned work package stays inside Workstream 6: either the macro layer
  on top of the new canonical app contract or the next additional hosted app,
  with Workstream 7 still treated as the design-first rack-scaling direction.

## Recommended Order of Refinements and Implementation

| Order | Workstream | Primary owner slice | Parallel-safe with | Blockers | Mandatory tests | Completion signal |
|---|---|---|---|---|---|---|
| 1 | Toolchain and verification baseline restoration | Worker 3 + Main Integrator | 2 | Working `cmake`, `ctest`, `make`, and expected build-path discovery. | Doc consistency checks in the current iteration; once toolchain access is restored, rerun the documented DaisyHost host gate and relevant firmware `make`. | Fresh baseline commands are rerunnable from this checkout without path repair guesswork. |
| 2 | Documentation and verification-truth reconciliation | Worker 3 | 1 | Reliable runtime evidence from workstream 1 for any new runtime claims. | Doc consistency checks across `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, and `LATEST_PROJECTS.md`. | Local docs agree on tracker/testing policy and no stale runtime counts or artifact assertions remain unlabeled. |
| 3 | Automated standalone/render smoke harness | Main Integrator | 2 | 1 | Targeted host tests, render smoke runs, standalone smoke validation, and the full DaisyHost host gate before milestone handoff. | Repeatable standalone/render smoke coverage is automated and documented in the tracker/checkpoint. |
| 4 | JUCE/DAW canonical parameter bridge | Main Integrator | 5 after parameter/readback contract is stable | 1, 3 | Targeted processor/session/render tests plus the full DaisyHost host gate. | Canonical DAW-visible parameter exposure is stable across app selection and session restore. |
| 5 | Effective-state inspection/readback API | Main Integrator | 4 after parameter IDs are stable | 1, 3 | Targeted processor/session/input/render tests plus the full DaisyHost host gate. | Effective parameter, CV, gate, and input state can be read back without ambiguity from host automation paths. |
| 6 | Additional hosted apps and macro layer | Worker 1 + Main Integrator | 2, 7 | 4, 5 | New app or macro targeted tests, relevant firmware `make`, and the full DaisyHost host gate. | A new hosted app or macro layer lands without processor/editor rewrites or contract drift. |
| 7 | Multi-instance rack direction | Worker 3 + Main Integrator | 2 | 4, 5, 6 for productionization | Diagram refresh, design review, and node-scoped targeted tests if implementation starts. | Multi-node rack direction is documented with safe joins, dependencies, and slice boundaries before implementation begins. |

## Parallel Thread Coordination

### Workstream Claim Rules

- Claim the active workstream in the per-iteration ledger before editing if
  another thread could overlap.
- If the workstream changes mid-iteration, append a new ledger row instead of
  silently mutating the old one.

### Slice Boundaries

- `AGENTS.md` is the source of truth for ownership slices and shared behavior
  contracts.
- `PROJECT_TRACKER.md` is the source of truth for active work order,
  per-iteration testing entries, and handoffs.
- Stay inside the assigned slice unless an integrator or the user explicitly
  reassigns the work.

### Handoff Rules

- Every handoff must include: files or slice touched, exact tests run, docs
  reviewed, blockers, and the next safe starting point.
- If docs changed, list which high-priority tracking files were reviewed and
  which ones were updated.

### Cross-Thread Conflict Rule

- If a planned edit touches another active slice, stop and coordinate in the
  tracker before changing the file.
- No cross-slice edits without reassignment or an explicit integration step.

### Per-Iteration Ledger

| Date | Thread / agent | Workstream | Files / slice | Tests run | Docs reviewed | Blockers | Handoff |
|---|---|---|---|---|---|---|---|
| 2026-04-22 | Local Codex thread | Workstream 6: first additional hosted app pilot (`cloudseed`) | `include/daisyhost/DaisyCloudSeedCore.h`, `src/DaisyCloudSeedCore.cpp`, `include/daisyhost/apps/CloudSeedCore.h`, `src/apps/CloudSeedCore.cpp`, `src/AppRegistry.cpp`, `src/juce/DaisyHostPluginProcessor.cpp`, `CMakeLists.txt`, `tests/test_daisy_cloudseed_core.cpp`, `tests/test_cloudseed_core.cpp`, `tests/test_app_registry.cpp`, `tests/test_host_automation_bridge.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/cloudseed_smoke.json`, `../third_party/CloudSeedCore/DSP/RandomBuffer.cpp`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red: `Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target unit_tests` (first failed for missing `daisyhost/DaisyCloudSeedCore.h` and `daisyhost/apps/CloudSeedCore.h`; later failed on the upstream `RandomBuffer.cpp` VLA under MSVC). Targeted green: `Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target unit_tests`; `Remove-Item Env:PATH -ErrorAction SilentlyContinue; ctest --test-dir build -C Release --output-on-failure -R "(AppRegistryTest|HostAutomationBridgeTest|DaisyCloudSeedCoreTest|CloudSeedCoreTest|RenderRuntimeTest)"` (`24/24` passed). Full gate: `Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`; `Remove-Item Env:PATH -ErrorAction SilentlyContinue; ctest --test-dir build -C Release --output-on-failure` (`79/79` passed, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`). | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild-backed commands still need the sanitized one-path form above; upstream `CloudSeedCore` also needed a local MSVC portability fix in `RandomBuffer.cpp` because it used a VLA. | The first Workstream-6 app pilot is now source-backed and green through the full host gate. What this unlocks: DaisyHost can now host a premium stereo reverb app on top of a portable shared wrapper, with page-based Patch controls, stable automation priorities, effective-state visibility, and a checked-in render smoke scenario ready for Hub/Render flows. |
| 2026-04-22 | Local Codex thread | Verification workflow hardening + firmware parity rerun | `build_host.ps1`, `build_host.cmd`, `tools/write_vst3_manifest.ps1`, `CMakeLists.txt`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red/debug evidence: `powershell -ExecutionPolicy Bypass -File .\build_host.ps1` initially failed on a script parser error, then on duplicate env-key lookup, then exposed a VST3 post-build failure; targeted repro: direct `juce_vst3_helper.exe` succeeded from PowerShell but failed through the MSBuild/`cmd` path with `LoadLibraryW failed for path ""`; green verification: `.\build_host.cmd` (`71/71` passed), `make` in `patch/MultiDelay/` (passed), `make` in `patch/Torus/` (up-to-date), PowerShell removal of `patch/Torus/build`, then `make` in `patch/Torus/` (passed fresh rebuild). | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | External DAW/VST3 load validation remains intentionally deferred until post-WS7; `patch/Torus` clean rebuild still needs PowerShell `build/` removal on Windows because `make clean` delegates to `rm`. | The local host gate now has a standard wrapper entrypoint and fresh firmware proof. What this unlocks: repeatable checkout verification no longer depends on remembering the shell sanitization ritual, and firmware parity is re-proven from the current checkout before Workstream 6 starts. |
| 2026-04-22 | Local Codex thread | Parallel Workstreams 4 and 5: DAW bridge + effective-state readback | `include/daisyhost/HostAutomationBridge.h`, `src/HostAutomationBridge.cpp`, `include/daisyhost/EffectiveHostStateSnapshot.h`, `src/EffectiveHostStateSnapshot.cpp`, `src/juce/DaisyHostPluginProcessor.h`, `src/juce/DaisyHostPluginProcessor.cpp`, `tests/test_host_automation_bridge.cpp`, `tests/test_effective_host_state_snapshot.cpp`, `CMakeLists.txt`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `README.md`, `CHANGELOG.md` | Red: `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake -S . -B build; cmake --build build --config Release --target unit_tests` (expected unresolved helper symbols before implementation). Targeted green: `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake --build build --config Release --target unit_tests; ctest --test-dir build -C Release -R "(HostAutomationBridgeTest|EffectiveHostStateSnapshotTest)" --output-on-failure` (`6/6` passed). Full gate: `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake -S . -B build; cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone; ctest --test-dir build -C Release --output-on-failure` (`71/71` passed). | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Codex PowerShell still exports both `Path` and `PATH`; the first combined target rebuild also needed a longer timeout during the standalone leg, but the rerun completed cleanly. | W4/W5 are now source-backed and verified from this checkout. What this unlocks: DAW automation can target five stable canonical slots across app switches, and host/debug tooling can read back live parameter/CV/gate/audio-input state through the processor. Next safe starting point is Workstream 6 or manual DAW-side validation of the new slot bridge. |
| 2026-04-22 | Local Codex thread | Parallel Workstreams 4 and 5: DAW bridge + effective-state readback | Claimed main-integrator host plumbing plus required tracking docs before code start | Pre-implementation claim only; red test/build evidence will be logged in this same iteration before closeout | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | None at claim time | Active package is now the combined W4/W5 implementation: stable five-slot DAW automation bridge plus processor-side effective-state snapshot/readback. |
| 2026-04-22 | Local Codex thread | Automated standalone/render smoke harness | `CMakeLists.txt`, `tests/run_smoke.py`, `README.md`, `training/README.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red step: `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake -S . -B build; ctest --test-dir build -C Release -R "DaisyHost(Standalone|Render)Smoke" --output-on-failure` (expected fail with placeholder harness). Green/full verification: `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake -S . -B build; cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone; ctest --test-dir build -C Release --output-on-failure` | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `training/README.md` | Codex PowerShell still exports both `Path` and `PATH`; the build step still needs the sanitized form above in this shell | Workstream 3 is now landed: `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke` are part of the host gate. Next planned work package is workstream 4, the JUCE/DAW canonical parameter bridge. |
| 2026-04-22 | Local Codex thread | Toolchain and verification baseline restoration | verification only plus `PROJECT_TRACKER.md` and `CHECKPOINT.md` | `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone; ctest --test-dir build -C Release --output-on-failure` | `PROJECT_TRACKER.md`, `CHECKPOINT.md` | Codex PowerShell still exports both `Path` and `PATH`; use the sanitized form above for MSBuild in this shell | Full DaisyHost host gate now passes in this checkout; next thread can move on to the next planned iteration instead of more baseline repair. |
| 2026-04-22 | Local Codex thread | Toolchain and verification baseline restoration | verification only plus `PROJECT_TRACKER.md` and `CHECKPOINT.md` | `Get-Command cmake`; `Get-Command make`; `cmake -S . -B build`; `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake --build build --config Release --target daisyhost_core`; `$env:Path = $env:PATH; Remove-Item Env:PATH; cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`; `ctest --test-dir build -C Release --output-on-failure` | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Default shell exports both `Path` and `PATH`, which breaks MSBuild unless normalized first; `DaisyHostPatch_Standalone` relink was blocked because `build\\DaisyHostPatch_artefacts\\Release\\Standalone\\DaisyHost Patch.exe` was already running and locked | Next thread can finish the full host gate by closing the running standalone app and rerunning the sanitized build command for `DaisyHostPatch_Standalone`; no path-discovery repair is needed anymore. |
| 2026-04-22 | Local Codex thread | Documentation and verification-truth reconciliation | `DaisyHost` tracking docs and roadmap artifacts | `d2 validate DaisyHost/ROADMAP.d2`; `d2 DaisyHost/ROADMAP.d2 DaisyHost/ROADMAP.svg`; doc consistency review across DaisyHost tracking files | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `LATEST_PROJECTS.md` | `cmake` and `make` unavailable on `PATH`; `DaisyHost/build` absent, so no fresh runtime build/test rerun was possible | Next thread should treat local runtime verification as historical until the documented host gate and relevant firmware builds are rerun. |
| 2026-04-22 | Local Codex thread | Roadmap display-readability pass | `ROADMAP.d2`, `ROADMAP.svg`, `PROJECT_TRACKER.md` | `d2 validate DaisyHost/ROADMAP.d2`; `d2 DaisyHost/ROADMAP.d2 DaisyHost/ROADMAP.svg` | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md` | None for the docs artifact pass | Roadmap layout is now stacked for on-screen viewing; rerun the same `d2` commands after any future roadmap edit. |
| 2026-04-22 | Local Codex thread | Skill playbook extraction and UF-method cleanup | `SKILL_PLAYBOOK.md`, `PROJECT_TRACKER.md`, `README.md`, `AGENTS.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `LATEST_PROJECTS.md` | Doc consistency review for `SKILL_PLAYBOOK.md` references and `Expected UF` / `Observed UF` terminology | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `LATEST_PROJECTS.md` | Runtime toolchain still unavailable in this checkout; skill ratings remain mostly expected until more DaisyHost task evidence is logged | Next thread should log materially used skills in `SKILL_PLAYBOOK.md` and update `Observed UF` only from actual DaisyHost evidence. |

## Skill Playbook

Skill-related activities now live in [SKILL_PLAYBOOK.md](SKILL_PLAYBOOK.md).

Interpretation rules:

- `Expected UF` = prior expected usefulness for DaisyHost tasks of the relevant
  class; it is not validated by description alone.
- `Observed UF` = evidence-based usefulness from actual DaisyHost task use.
- Update `Observed UF` only when a skill was materially used and the outcome was
  logged with concrete evidence.

## Roadmap Diagram

- [ROADMAP.d2](ROADMAP.d2)
- [ROADMAP.svg](ROADMAP.svg)

The D2 source and compiled SVG visualize, in a stacked screen-friendly layout:

- governance and tracking documents
- the required iteration loop
- the recommended workstream order
- safe parallel ownership joins

## Currently Working Functions

| Function | What it does | How it's done |
|---|---|---|
| Hosted app registry | Registers available DaisyHost apps and resolves a default app when an unknown id is requested. | `src/AppRegistry.cpp` registers `multidelay`, `torus`, and `cloudseed`; `tests/test_app_registry.cpp` covers registration and unknown-id fallback. |
| Patch board profile | Models the virtual Daisy Patch surface, control hierarchy, ports, and panel artwork metadata. | `src/BoardProfile.cpp` builds the profile; `tests/test_board_profile.cpp` checks port counts, Patch-like placement, artwork markers, and the final `CTRL 1..4 + ENC` mapping. |
| MultiDelay shared core/menu model | Exposes canonical parameters, OLED/menu interaction, Patch bindings, and deterministic DSP state for the regression fixture app. | `src/apps/MultiDelayCore.cpp` plus `tests/test_multidelay_core.cpp` and `tests/test_parameter_parity.cpp` cover menu editing, `last touch wins`, control mapping, state capture/restore, and menu-vs-direct parameter parity. |
| Torus hosted app | Provides a second hosted app with stable metadata, display/menu naming, and deterministic render behavior. | `src/apps/TorusCore.cpp`; `tests/test_torus_core.cpp` covers metadata/menu, deterministic reset/render, silent boot without excitation, and display compaction rules. |
| CloudSeed hosted app pilot | Hosts a portable stereo reverb wrapper with performance-page knob remapping, utility actions, and deterministic render state. | `src/DaisyCloudSeedCore.cpp` owns the portable CloudSeed mapping/state layer; `src/apps/CloudSeedCore.cpp` exposes the DaisyHost adapter and menu/display model; `tests/test_daisy_cloudseed_core.cpp`, `tests/test_cloudseed_core.cpp`, and `training/examples/cloudseed_smoke.json` cover the shared core, hosted app contract, render flow, and smoke scenario. |
| Host session persistence | Persists app selection, canonical parameter values, MIDI learn bindings, and host-side generator settings across save/load. | `src/HostSessionState.cpp`; `tests/test_host_session_state.cpp` covers v1/v2/v3 session round trips, app id persistence, MIDI learn state, and host generator controls. |
| Headless render runtime | Loads scenario JSON, validates event timelines, renders offline audio, and writes manifest output. | `tools/render_app.cpp` and `src/RenderRuntime.cpp`; `tests/test_render_runtime.cpp` covers parsing, validation, deterministic render, event ordering, Torus smoke render, and `audio.wav` + `manifest.json` output. |
| Automated standalone/render smoke harness | Launches the real standalone app for startup-stability smoke and exercises the real render CLI against checked-in smoke scenarios. | `tests/run_smoke.py` runs direct-entrypoint smoke checks; CTest wires it in as `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`, including repeated `multidelay` checksum comparison plus checked-in `torus` and `cloudseed` render scenarios. |
| DAW-visible canonical parameter bridge | Exposes a fixed five-slot JUCE/VST automation bank with stable ids and active-app rebinding by canonical parameter metadata. | `src/HostAutomationBridge.cpp` ranks automatable parameters into `daisyhost.slot1`..`daisyhost.slot5`; `src/juce/DaisyHostPluginProcessor.cpp` owns the five JUCE parameters, app-switch/session-restoration rebinding, and DAW-write application back into canonical app parameters. |
| Effective-state inspection/readback API | Builds a processor-side snapshot of live canonical parameters, mapped automation slots, CV state, gate state, and current audio-input configuration. | `src/EffectiveHostStateSnapshot.cpp` assembles the shared snapshot model; `DaisyHostPatchAudioProcessor::GetEffectiveHostStateSnapshot()` exposes it from live processor state. |
| Dataset orchestration and hub launch planning | Expands dataset jobs and prepares `Play / Test`, `Render`, and `Train` launch plans around the same render/training entrypoints. | `training/render_dataset.py`, `src/HubSupport.cpp`, and `src/hub/*`; `tests/test_hub_support.cpp` covers launch planning, startup handoff, profile persistence, generated scenario/job creation, and tool-path discovery. |
| MIDI interaction path | Supports computer-keyboard note entry, MIDI event tracking, note preview, and CC-learn plumbing in the host processor. | `src/ComputerKeyboardMidi.cpp`, `src/MidiEventTracker.cpp`, `src/MidiNotePreview.cpp`, and `src/juce/DaisyHostPluginProcessor.cpp`; tests cover keyboard mapping and tracker formatting while processor code owns learn/binding state. |
| Version/About surfacing | Exposes the current version, build identity, and release highlights to the host UI/About flow. | `project(DaisyHost VERSION 0.2.0 ...)` in `CMakeLists.txt`, `src/VersionInfo.cpp`, and `tests/test_version_info.cpp`. |

## Refinements Worth Doing

| Function | What it should do | What is current limitations |
|---|---|---|
| Documentation alignment | Make local docs agree on what is actually verified in this checkout. | The host gate and direct-entrypoint smoke harness are now rerun from this checkout, but manual DAW/runtime checks and any future firmware parity reruns still need their own dated evidence. |
| Repeatable checkout verification | Let a fresh checkout rerun the documented validation commands without manual environment repair. | `build_host.cmd` now encapsulates the local `Path` / `PATH` repair and the full host gate, but anyone bypassing the wrapper and running raw MSBuild-backed commands still needs to sanitize the shell manually. |
| DAW and standalone smoke validation | Reconfirm plugin load, GUI behavior, and hub dispatch against current binaries. | Direct-entrypoint standalone startup stability and render smoke are now automated, but DAW-side `VST3` load, exhaustive GUI click-through, and hub manual pass remain intentionally deferred until post-WS7. |
| DAW automation bridge expansion | Extend the new five-slot bank into richer host labeling, broader parameter coverage, or future app-specific surfaces without breaking saved automation. | W4 now lands the fixed five-slot bridge, but DAW-visible names remain generic and only the top five automatable parameters are surfaced in v1. |
| Patch firmware parity check | Reconfirm shared-core behavior against `patch/MultiDelay/` and `patch/Torus/` from the current checkout. | Fresh `make` passes are now recorded for both firmware references, but clean rebuilds on Windows still need project-specific handling because `patch/Torus` `make clean` delegates to `rm`. |
| External readback/export surface | Extend the new in-process snapshot API into an export, debug, or tooling surface outside the processor ABI. | W5 now lands the processor-side snapshot, but no JSON, CLI, or IPC surface exists yet for external tools or DAW-adjacent diagnostics. |
| Doc and roadmap freshness | Keep design notes aligned with implemented platform features. | The new roadmap artifacts now exist in DaisyHost, but related historical plan docs under `docs/plans/` still need human review before they are treated as current. |

## Functionalities Worth Implementing

| Function | What it should do | Why it worth implementing |
|---|---|---|
| Expanded DAW automation surface | Grow the new five-slot automation bridge into richer labels, more coverage, or a follow-on bank without breaking the saved-v1 ids. | The stable slot bridge is now present, so the next value is breadth and DAW ergonomics rather than initial exposure. |
| Hub GUI / DAW smoke automation | Extend the new smoke harness from direct entrypoints into Hub dispatch and DAW-side plugin load coverage. | The direct standalone/render smoke baseline is now in place, so the next automation value is the still-manual Hub and VST3 validation path. |
| External effective-state tooling surface | Take the new processor-side snapshot/readback model and expose it to external tools, diagnostics, or future automation surfaces. | The live state model now exists in-process, so the next step is making it consumable outside the processor/UI boundary. |
| Additional hosted apps with reusable metadata contracts | Add more first-class hosted apps without editor or processor rewrites. | The registry/factory pattern is already in place, so expanding app coverage increases platform value faster than more UI polish alone. |
| Multi-instance rack runtime | Host more than one Daisy app or node in the same process with routable audio, CV, gate, and MIDI paths. | Current abstractions are already node-scoped, so this is the natural scaling path after single-app stability. |
| MetaController / macro layer | Add deterministic semantic controls that map one macro to multiple canonical parameters. | It is a higher-leverage extension than another fixed demo app and matches the documented platform-first roadmap. |

## Iteration Log

| Date | Confirmed working in this iteration | Problems observed in this iteration |
|---|---|---|
| 2026-04-22 | Workstream-6 pilot landed from this checkout: DaisyHost now hosts `cloudseed` on top of a new portable `DaisyCloudSeedCore`, page-based `Space` / `Motion` knob remapping is live in the hosted app and the JUCE processor binding refresh, `cloudseed_smoke.json` is checked in, and the full host gate reran green at `79/79`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must still be run in a sanitized one-path shell; upstream `CloudSeedCore` also needed a local MSVC portability fix because `RandomBuffer.cpp` used a VLA. |
| 2026-04-22 | `build_host.cmd` now reruns the full DaisyHost host gate from this checkout, `build_host.ps1` encapsulates the local `Path` / `PATH` normalization, the VST3 manifest step is now PowerShell-backed and rerunnable on Windows, and fresh firmware reference builds passed in `patch/MultiDelay/` and `patch/Torus/`. | External DAW/VST3 load validation is intentionally deferred until post-WS7; `patch/Torus` still needs a PowerShell `build/` removal for a clean rebuild because `make clean` calls `rm` on Windows. |
| 2026-04-22 | Workstreams 4 and 5 landed from this checkout: DaisyHost now builds a fixed five-slot DAW automation bridge plus an in-process effective-state snapshot API, `unit_tests` gained bridge/readback coverage, and the full host gate reran successfully with `71/71` tests green. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell; external DAW/VST3 load validation is still manual. |
| 2026-04-22 | `tests/run_smoke.py` landed and the full DaisyHost host gate reran successfully from this checkout: `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone` rebuilt in `Release`, then `ctest` passed `65/65`, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell. |
| 2026-04-22 | Full DaisyHost host gate reran successfully from this checkout: `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone` rebuilt in `Release`, then `ctest` passed `63/63`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell. |
| 2026-04-22 | Runtime verification partially restored from this checkout: `cmake -S . -B build` succeeded, `unit_tests` built and `ctest` passed `63/63`, and `DaisyHostHub`, `DaisyHostRender`, and `DaisyHostPatch_VST3` rebuilt successfully. | The default shell exports both `Path` and `PATH`, which breaks MSBuild until normalized, and the running `build\\DaisyHostPatch_artefacts\\Release\\Standalone\\DaisyHost Patch.exe` blocked the standalone relink with `LNK1104`. |
| 2026-04-22 | Documentation/governance hardening completed: `PROJECT_TRACKER.md` promoted to first-class tracking doc; `README.md`, `AGENTS.md`, `CHECKPOINT.md`, `CHANGELOG.md`, and `LATEST_PROJECTS.md` aligned around tracker usage and mandatory per-iteration testing; `ROADMAP.d2` and `ROADMAP.svg` added. | `cmake` and `make` remain unavailable on `PATH`, and `DaisyHost/build` is absent, so runtime build/test claims remain historical until rerun. |
| 2026-04-22 | `ROADMAP.d2` was restructured into stacked governance, workstream, iteration, and ownership bands and recompiled to a more preview-friendly `ROADMAP.svg`. | Runtime build/test tooling is still outside the scope of this docs-only pass, so no new host or firmware verification was attempted. |
| 2026-04-22 | `SKILL_PLAYBOOK.md` was added as the dedicated DaisyHost file for skill-related activities, and the tracker now distinguishes `Expected UF` from evidence-based `Observed UF`. | Most skills still have no DaisyHost-local observed sample set, so `Observed UF` remains pending until future task iterations log real evidence. |
