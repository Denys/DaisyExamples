# DaisyHost AGENTS

Local agent instructions for `DaisyHost/`.

Read this file after the repo-level [AGENTS.md](../AGENTS.md) and before
editing `DaisyHost/`.

## Start Here

1. Read [README.md](README.md) for the workspace overview.
2. Read [CHECKPOINT.md](CHECKPOINT.md) for the current verified state, last
   recorded runtime verification date, control map, and open issues.
3. Read [PROJECT_TRACKER.md](PROJECT_TRACKER.md) for the current work order,
   per-iteration testing ledger, and concurrent-thread status.
4. Read [SKILL_PLAYBOOK.md](SKILL_PLAYBOOK.md) when the task depends on skill
   selection, workflow reuse, or `Expected UF` / `Observed UF` updates.
5. Read [CHANGELOG.md](CHANGELOG.md) before editing user-facing behavior or
   durable workflow/release notes.
6. If multiple agents are active, stay inside your ownership slice. Do not
   touch another slice's files unless the user explicitly reassigns ownership
   or an integrator asks for a conflict-resolution edit.

## Manager-Language Planning And Closeout

- Lead DaisyHost planning and completion notes in manager terms before
  implementation detail.
- During planning, state what needs to be implemented, why it matters, what it
  unlocks, what depends on it, what is out of scope, and what evidence will
  prove it is done.
- During implementation closeout, state what was implemented, why it matters,
  what changed for users/agents/CI, what remains, what was explicitly not
  claimed, and the verification evidence.
- Technical details still belong in the tracker, but they support the decision
  story rather than replacing it.
- If a plan or handoff is only understandable by reading file names, class
  names, or test names, rewrite it before treating the iteration as handed off.

## Workspace Map

- `include/daisyhost/`: shared host-side interfaces and data models
- `src/apps/`: portable extracted Daisy app cores
- `src/juce/`: JUCE processor/editor implementation
- `tests/`: host-side unit coverage
- `build/`: generated host-side outputs; use it for artifact checks, not as source of truth

## Parallel Ownership Slices

### Worker 1: Core, Parameters, Menu, Firmware

Owns:

- `include/daisyhost/HostedAppCore.h`
- `include/daisyhost/LiveRackTopology.h`
- `include/daisyhost/DisplayModel.h`
- `include/daisyhost/apps/MultiDelayCore.h`
- `src/LiveRackTopology.cpp`
- `src/apps/MultiDelayCore.cpp`
- `tests/test_live_rack_topology.cpp`
- `tests/test_multidelay_core.cpp`
- `../patch/MultiDelay/MultiDelay.cpp`

Expected outcomes:

- shared `ParameterDescriptor` and `MenuModel`
- live-rack topology preset expansion and reverse mapping helpers
- final Patch control hierarchy
- encoder/menu behavior shared by host and firmware
- menu-editable access to all five DSP parameters
- `last touch wins` arbitration across knob, CV, and menu sources

Required verification before handoff:

- targeted live-rack topology tests pass
- targeted `MultiDelayCore` tests pass
- `make` passes in `../patch/MultiDelay`

### Worker 2: Patch UI, Board Layout, Icon, Visual Fidelity

Owns:

- `include/daisyhost/BoardProfile.h`
- `src/BoardProfile.cpp`
- `src/juce/DaisyHostPluginEditor.h`
- `src/juce/DaisyHostPluginEditor.cpp`
- `assets/*`
- `tests/test_board_profile.cpp`

Expected outcomes:

- Patch-faithful vector layout
- visible 2-node rack header and topology visuals without panel-layout regressions
- visible control positions that match the final hierarchy
- reduced right-hand drawer that mirrors the shared menu model
- board-profile factory seam for board-aware UI/runtime handoff
- standalone icon assets

Required verification before handoff:

- board-profile tests pass
- standalone app launches and renders without layout corruption

### Worker 3: Repo Awareness, Docs, Version Tracking

Owns:

- `../AGENTS.md`
- `../LATEST_PROJECTS.md`
- `AGENTS.md`
- `PROJECT_TRACKER.md`
- `SKILL_PLAYBOOK.md`
- `CHECKPOINT.md`
- `CHANGELOG.md`
- `README.md`

Expected outcomes:

- DaisyHost treated as a pinned first-party workspace
- local docs for current state, parallel ownership, ordered implementation, and
  known issues
- docs aligned with actual artifact names and build targets already present in the repo

Required verification before handoff:

- reread all eight owned docs for consistency
- confirm `PROJECT_TRACKER.md` reflects the current work order, testing ledger,
  and doc-review cadence
- confirm `SKILL_PLAYBOOK.md` reflects any materially used skills or UF updates
- confirm the host test target remains `unit_tests`
- do not claim code behavior or runtime verification that was not rerun in the
  current iteration

### Main Integrator: Host Plumbing, Inputs, Version Surfacing

Owns:

- `CMakeLists.txt`
- `src/juce/DaisyHostPluginProcessor.h`
- `src/juce/DaisyHostPluginProcessor.cpp`
- `include/daisyhost/HostSessionState.h`
- `src/HostSessionState.cpp`
- `include/daisyhost/RenderTypes.h`
- `include/daisyhost/EffectiveHostStateSnapshot.h`
- `src/EffectiveHostStateSnapshot.cpp`
- `src/RenderRuntime.cpp`
- `include/daisyhost/HostStartupPolicy.h`
- `src/HostStartupPolicy.cpp`
- `tests/test_host_startup_policy.cpp`
- new targeted tests for rack runtime, render/session wiring, or snapshot expansion

Expected outcomes:

- `0.2.0` version bump
- `Saw` test input and `Host In` startup default
- version/about surfacing in the app
- final integration between shared menu state and the mirrored host drawer
- visible 2-node live rack runtime with selected-node targeting
- rack-aware session/render/snapshot plumbing
- board-selection seam consumed by the runtime and hub-side flows

Required verification before handoff:

- startup-policy tests pass
- saw-input behavior is tested
- session/version wiring is tested or manually validated

## Mandatory Per-Iteration Testing

- Every implementation iteration must record the exact checks run, date, result,
  and affected slice in `PROJECT_TRACKER.md`. If a skill was materially used or
  re-evaluated, also update `SKILL_PLAYBOOK.md`. No iteration is complete
  without test evidence.
- Every planned or completed WP must include manager-readable explanation
  alongside technical detail. Record what is being implemented or what was
  done, why it matters, what it unlocks, what remains, and what is explicitly
  out of scope so a non-specialist can track progress without knowing the
  internals.
- After a WP/workstream implementation is complete enough for handoff, run the
  next-WP recommender and record the recommended WP, runner-up, explicit waits,
  and first safe slice in the `PROJECT_TRACKER.md` handoff:

```sh
py -3 tools/suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md
```

- For code changes, follow test-first discipline: write a failing test first,
  verify the failure, implement the minimal change, rerun targeted tests, then
  update the tracker/docs.
- Docs-only iterations must still run doc consistency checks. If `ROADMAP.d2`
  or its references changed, also run:

```sh
d2 validate DaisyHost/ROADMAP.d2
d2 DaisyHost/ROADMAP.d2 DaisyHost/ROADMAP.svg
```

- Every code iteration must run targeted tests for the touched slice before
  handoff:
  - Worker 1: targeted `MultiDelayCore` tests; run `make` in `../patch/MultiDelay`
    and/or `../patch/Torus` when shared-core or firmware-facing surfaces change
  - Worker 2: board/UI-targeted tests plus standalone smoke validation when UI
    behavior changed
  - Main Integrator: targeted host/session/input/render tests for the touched
    processor/session surfaces
- Full DaisyHost host gate is required before milestone or merge handoff:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

- If shared-core or firmware-facing code changed, also run the relevant firmware
  builds before milestone or merge handoff:

```sh
make
```

from `../patch/MultiDelay/` and/or `../patch/Torus/` as appropriate.

## Concurrent Thread Protocol

- `AGENTS.md` remains the source of truth for ownership slices and shared
  behavior contracts.
- `PROJECT_TRACKER.md` remains the source of truth for active work order,
  claimed workstreams, per-iteration testing entries, and handoff notes.
- Claim the workstream in `PROJECT_TRACKER.md` before editing if another thread
  could overlap.
- Stay inside your ownership slice. No cross-slice edits without reassignment or
  an explicit integration step.
- Every handoff must include the tests run, docs reviewed, blockers, and the
  next safe starting point.
- Every handoff must also include the manager-readable explanation for the WP:
  done/implemented, why it matters, remaining work, and explicit non-goals.
- For WP closeouts, "next safe starting point" must include the recommender's
  decision-ready next WP, runner-up, waits, dependency reason, overlap risk, and
  first safe implementation slice. If the recommender output conflicts with
  human judgment, record both and explain which source won.
- If a planned edit touches another active slice, stop and coordinate in
  `PROJECT_TRACKER.md` before changing the file.

## Shared Behavior Contract

- Final Patch control hierarchy:
  - `CTRL 1` = dry/wet mix
  - `CTRL 2` = primary delay control
  - `CTRL 3` = secondary delay control
  - `CTRL 4` = feedback
  - `CV 1` = tertiary delay control
  - `ENC 1` rotate + push = menu navigation and edit/confirm
- For current `MultiDelayCore`, the delay priority order is:
  - primary = old delay 1 target
  - secondary = old delay 2 target
  - tertiary = old delay 3 target
- Every DSP parameter must also be editable from the OLED menu.
- If knob, CV, or menu all target the same parameter, `last touch wins`.

## Completion Gate

Do not call the plugin complete until all of the following pass together:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

If shared-core or firmware-facing code changed, also run:

```sh
make
```

from `../patch/MultiDelay/` and/or `../patch/Torus/` as appropriate.

Manual validation still required before final handoff:

- standalone launch stability
- VST3 load in a DAW
- standalone icon visible
- `Host In`, `Sine`, `Saw`, `Noise`, and `Impulse`
- mouse control of all visible controls
- computer-keyboard MIDI
- external MIDI note input
- CC learn
- MIDI tracker updates
- encoder menu and drawer sync
- all five DSP parameters editable from both menu and live controls
- host and Patch firmware menu behavior aligned at the feature level
