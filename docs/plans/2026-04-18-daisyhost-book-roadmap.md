# DaisyHost Book Roadmap Implementation Plan

**Goal:** Extend `DaisyHost` from a single test-algorithm host into the minimum practical platform needed to realize the useful ideas from *Build AI-Enhanced Audio Plugins with C++* across desktop pre-flash testing, offline training/rendering, and Daisy hardware deployment.

**Architecture:** Keep a strict three-layer split:

- shared portable app core
- desktop host adapter
- Daisy firmware adapter

Reinterpret the book's desktop-only host ideas for Daisy instead of attempting literal 1:1 reproduction. Training remains offline in Python. Embedded inference remains small, deterministic, and RT-safe.

**Core insight added after review:** before MetaController training makes sense, `DaisyHost` must become a deterministic parameterized engine with a canonical parameter contract, direct automation APIs, state reset/serialization, effective-state readback, parity tests, and headless rendering. VST parameters are a useful bridge, not the primary abstraction.

**Tech Stack:** C++17, JUCE, DaisyHost shared core, libDaisy, DaisySP, RTNeural, Python, PyTorch, CMake, Make, GoogleTest.

---

## Scope And Success Criteria

This roadmap is specifically about implementing the **useful functionality** from the book repo:

- Part 2: SuperKnobs and MetaControllers
- Part 3: Markov improviser / MIDI generation
- Part 4: Neural FX with offline training and desktop/embedded inference

Out of scope for this roadmap:

- full STM32 or hardware emulation
- VST hosting on Daisy hardware
- literal reproduction of every JUCE book example
- multi-node Daisy rack in the first delivery

Success means:

1. `DaisyHost` can host multiple shared-core apps over time, not only `MultiDelay`.
2. `DaisyHost` exposes a canonical parameter model with deterministic reset, direct set/get APIs, state serialization, and effective-state readback.
3. A headless render path exists for reproducible dataset generation and automation.
4. At least one SuperKnob-style app, one Markov-style app, and one Neural-FX-style app exist in desktop host form.
5. At least the Markov and Neural-FX paths have Daisy deployment-ready adapters.
6. Training/export is reproducible offline and does not depend on running `libtorch` on Daisy.

## Current Starting Point

Use these as current anchors before implementation:

- `DaisyHost/include/daisyhost/HostedAppCore.h`
- `DaisyHost/include/daisyhost/apps/MultiDelayCore.h`
- `DaisyHost/src/apps/MultiDelayCore.cpp`
- `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- `DaisyHost/src/juce/DaisyHostPluginEditor.cpp`
- `patch/MultiDelay/MultiDelay.cpp`
- `DaisyHost/CHECKPOINT.md`

These prove the repo already has:

- a shared core contract
- menu/display models
- a board-profile driven Patch host
- a working desktop plugin + standalone build
- a firmware adapter path

What it does **not** yet fully prove is that DaisyHost is training-ready. The current state is still closer to an internal snapshot-driven host than a deterministic automation-ready engine contract.

## Source Material To Implement Against

Use the book repo as conceptual source, not as a literal file-copy target:

- Part 2: `https://github.com/yeeking/ai-enhanced-audio-book/tree/main/src/Part2_MetaController`
- Part 3: `https://github.com/yeeking/ai-enhanced-audio-book/tree/main/src/Part3_Improviser`
- Part 4: `https://github.com/yeeking/ai-enhanced-audio-book/tree/main/src/Part4_NeuralFX`

The most relevant book examples are:

- `007_fmplugin_superknob`
- `010e_metacontroller`
- `MarkovModelCPP`
- `020i_midi_markov_time`
- `037a_train_lstm`
- `037d_lstm-rtneural`
- `037e_lstm-rtneural-JUCE`

## MetaController Sweet Spot

Preserve the useful idea from the book:

- a compact semantic controller layer that can drive richer internal parameter spaces
- macro mapping, scene control, and eventually learned mapping

Do **not** preserve the part that does not fit Daisy well:

- desktop plugin hosting as the runtime dependency for the controller
- replaying board UI gestures as the training interface

The correct DaisyHost interpretation is:

- train and automate against **canonical internal parameters**, macro outputs, or scene controls
- let Patch knobs, encoder/menu, MIDI learn, and host GUI act as **clients** of that parameter model
- keep Daisy firmware and desktop host aligned through the shared core, not through desktop-host-only abstractions

## Canonical Control Contract

Before MetaController training, DaisyHost should expose a three-layer control contract.

### Layer 1: Canonical internal parameters

This is the source of truth.

Each parameter spec should eventually include:

- stable id
- label
- default value
- range and normalization policy
- optional units
- optional step or enum hints
- role and visibility metadata
- smoothing or takeover policy
- serialization support
- effective-value readback

### Layer 2: UI and physical control surfaces

These are clients of Layer 1:

- Patch knobs and CV
- encoder/menu navigation
- MIDI learn
- desktop host widgets
- scenes, macros, and metacontroller mappings

### Layer 3: External automation bridges

These are adapters, not the source of truth:

- JUCE or VST parameters
- headless render CLI
- dataset-generation scripts
- later DAW or DawDreamer automation

## Training Gate Before MetaController Training

MetaController training should **not** start until these capabilities exist.

1. **Canonical parameter specs**
   Every trainable input or target must exist as a stable internal parameter definition, even if only a subset is exposed as JUCE or VST parameters.

2. **Direct parameter API**
   Training and batch automation must be able to set and read values by parameter id without turning the encoder or navigating menus.

3. **Deterministic reset and full state serialization**
   It must be possible to reset the host to a known state, load and save the full state, fix random seeds, and re-render the same input reproducibly.

4. **Effective-state observation**
   The engine must expose the actual effective values after smoothing, clamping, macro mapping, scene morphing, and related transforms.

5. **Parity tests**
   Add tests for parameter mapping curves, smoothing, takeover, menu-vs-direct parameter equivalence where relevant, and host-vs-Daisy adapter behavior.

6. **Headless rendering**
   Add a GUI-free render path before serious training work begins.

Only after this gate is green does MetaController training become worth doing.

## Phase 1: Canonical Parameter Contract And Deterministic Core

### Task 1: Expand the shared parameter model into a real parameter spec

**Files:**

- Modify: `DaisyHost/include/daisyhost/HostedAppCore.h`
- Modify: `DaisyHost/include/daisyhost/DisplayModel.h`
- Modify: `DaisyHost/include/daisyhost/HostSessionState.h`
- Modify: `DaisyHost/src/HostSessionState.cpp`
- Test: `DaisyHost/tests/test_multidelay_core.cpp`

**Implement:**

- stable parameter ids independent of board control ids
- defaults, ranges, and normalization helpers
- metadata for visibility, role, and optional units or steps
- capability to enumerate parameter specs from any hosted app

**Validate:**

- parameters can be looked up by id
- parameter specs are stable across host launches
- app-core tests compile and pass

### Task 2: Add direct parameter APIs and deterministic reset/state flow

**Files:**

- Modify: `DaisyHost/include/daisyhost/HostedAppCore.h`
- Modify: `DaisyHost/include/daisyhost/apps/MultiDelayCore.h`
- Modify: `DaisyHost/src/apps/MultiDelayCore.cpp`
- Test: `DaisyHost/tests/test_multidelay_core.cpp`

**Implement:**

- `SetParameterValue(id, normalizedValue)`
- `GetParameterValue(id)`
- `GetEffectiveParameterValue(id)`
- `ResetToDefaultState()`
- full app state serialization and restore
- fixed-seed hooks where randomness exists

**Validate:**

- a known state can be saved and restored bit-for-bit at the app level
- menu edits and direct parameter sets converge on the same effective values when they target the same underlying parameter

### Task 3: Add deterministic parity checks for host and adapter behavior

**Files:**

- Modify: `DaisyHost/tests/test_multidelay_core.cpp`
- Create: `DaisyHost/tests/test_parameter_parity.cpp`

**Implement:**

- tests for smoothing and takeover behavior
- tests for direct-parameter vs menu-edit equivalence where meaningful
- audio-output tolerance checks for known inputs and fixed state

**Validate:**

- deterministic checks pass before continuing to multi-app work

## Phase 2: Generalize DaisyHost Beyond MultiDelay

### Task 4: Audit and harden the shared app-core contract

**Files:**

- Modify: `DaisyHost/include/daisyhost/HostedAppCore.h`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.h`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Test: `DaisyHost/tests/test_multidelay_core.cpp`

**Implement:**

- stable `GetAppId()`
- stable `GetAppDisplayName()`
- `GetCapabilities()` with booleans such as `acceptsAudioInput`, `acceptsMidiInput`, `producesMidiOutput`

Do not add rack graph or multi-instance logic yet.

### Task 5: Introduce an app registry and app selection model

**Files:**

- Create: `DaisyHost/include/daisyhost/AppRegistry.h`
- Create: `DaisyHost/src/AppRegistry.cpp`
- Modify: `DaisyHost/CMakeLists.txt`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Modify: `DaisyHost/src/juce/DaisyHostPluginEditor.cpp`
- Test: `DaisyHost/tests/test_app_registry.cpp`

**Implement:**

- simple factory registry for:
  - `MultiDelayCore`
  - future `SuperKnobDemoCore`
  - future `MarkovImproviserCore`
  - future `NeuralFxCore`

### Task 6: Add selective JUCE/VST host parameter exposure

**Files:**

- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.h`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Test: `DaisyHost/tests/test_host_parameter_bridge.cpp`

**Implement:**

- expose selected canonical parameters as JUCE or VST host parameters
- keep host parameters as an adapter layer over the canonical model
- do not expose raw menu navigation or every internal UI gesture

**Validate:**

- DAW automation can see the selected parameters
- the host parameter bridge does not become the source of truth

## Phase 3: Headless Rendering And Dataset Generation

### Task 7: Add a headless render path to DaisyHost

**Files:**

- Create: `DaisyHost/tools/render_app.cpp`
- Modify: `DaisyHost/CMakeLists.txt`
- Create: `DaisyHost/tests/test_render_app_smoke.cpp`

**Implement:**

- app selection by app id
- deterministic offline rendering
- GUI-free WAV output generation
- optional preset or state-load support

### Task 8: Add dataset-generation scripts for neural and control experiments

**Files:**

- Create: `DaisyHost/training/render_dataset.py`
- Create: `DaisyHost/training/configs/dataset_sweep.yaml`
- Create: `DaisyHost/training/README_DATASETS.md`

**Implement:**

- parameter sweep support
- metadata export for parameter values, presets, and seeds
- simple orchestration around the headless renderer

This phase must be green before MetaController training starts.

## Phase 4: Part 2 SuperKnob And MetaController

### Task 9: Implement a reusable macro-controller module

**Files:**

- Create: `DaisyHost/include/daisyhost/MacroController.h`
- Create: `DaisyHost/src/MacroController.cpp`
- Create: `DaisyHost/tests/test_macro_controller.cpp`

**Implement:**

- one macro value mapped to multiple target parameters
- per-target range and curve
- optional smoothing

This is the reusable Daisy interpretation of the book's SuperKnob concept.

### Task 10: Create a dedicated SuperKnob demo app

**Files:**

- Create: `DaisyHost/include/daisyhost/apps/SuperKnobDemoCore.h`
- Create: `DaisyHost/src/apps/SuperKnobDemoCore.cpp`
- Modify: `DaisyHost/include/daisyhost/AppRegistry.h`
- Modify: `DaisyHost/src/AppRegistry.cpp`
- Test: `DaisyHost/tests/test_superknob_demo_core.cpp`

**Implement:**

- one visible macro that changes several derived parameters
- derived parameters shown in menu pages
- display text updates correctly

### Task 11: Define the Daisy reinterpretation of MetaController

**Files:**

- Create: `docs/plans/2026-04-18-daisyhost-metacontroller-design.md`
- Create: `DaisyHost/include/daisyhost/MetaControllerModel.h`
- Create: `DaisyHost/src/MetaControllerModel.cpp`
- Create: `DaisyHost/tests/test_metacontroller_model.cpp`

**Implement:**

- semantic macro inputs
- internal parameter mapping
- optional model-driven mapping hook for future learned controllers

Do **not** implement desktop plugin hosting from the book. The target is a learned or handcrafted semantic controller over canonical internal parameters.

### Task 12: Add training-ready metacontroller experiments

**Files:**

- Create: `DaisyHost/training/train_metacontroller.py`
- Create: `DaisyHost/training/configs/metacontroller_small.yaml`
- Create: `DaisyHost/tests/test_metacontroller_training_contract.cpp`

**Implement:**

- training scripts that target canonical parameter ids or macro outputs
- no dependency on menu replay or board-specific encoder gestures
- a minimal contract test proving the trained controller can be exercised through the direct parameter and render interfaces

## Phase 5: Part 3 Improviser / Markov

### Task 13: Port the Markov model into DaisyHost shared core form

**Files:**

- Create: `DaisyHost/include/daisyhost/improviser/MarkovModel.h`
- Create: `DaisyHost/src/improviser/MarkovModel.cpp`
- Create: `DaisyHost/tests/test_markov_model.cpp`

**Implement:**

- reusable C++ Markov logic inspired by `MarkovModelCPP` and the `020c` through `020i` examples
- deterministic generation when seeded
- separable pitch, timing, and duration chains where useful

### Task 14: Create a Markov improviser host app

**Files:**

- Create: `DaisyHost/include/daisyhost/apps/MarkovImproviserCore.h`
- Create: `DaisyHost/src/apps/MarkovImproviserCore.cpp`
- Modify: `DaisyHost/src/AppRegistry.cpp`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Test: `DaisyHost/tests/test_markov_improviser_core.cpp`

**Implement:**

- MIDI input capture
- train buffer
- generated note output queue
- simple timing scheduler
- menu pages for train/generate/reset

### Task 15: Add a Daisy firmware adapter for the Markov app

**Files:**

- Create: `patch/MarkovImproviser/MarkovImproviser.cpp`
- Create: `patch/MarkovImproviser/Makefile`
- Create: `patch/MarkovImproviser/README.md`

**Implement:**

- shared core usage on Patch
- train/generate/reset controls
- MIDI input handling in the main loop
- optional OLED or LED feedback

## Phase 6: Part 4 Neural FX

### Task 16: Add a reproducible training workspace

**Files:**

- Create: `DaisyHost/training/README.md`
- Create: `DaisyHost/training/requirements.txt`
- Create: `DaisyHost/training/train_lstm.py`
- Create: `DaisyHost/training/export_rtneural.py`
- Create: `DaisyHost/training/configs/lstm_small.yaml`

**Implement:**

- training in Python
- export to RTNeural-friendly format
- one small model path only

Do not introduce embedded `libtorch`.

### Task 17: Add RTNeural inference wrapper to shared core

**Files:**

- Create: `DaisyHost/include/daisyhost/neural/NeuralModelRunner.h`
- Create: `DaisyHost/src/neural/NeuralModelRunner.cpp`
- Create: `DaisyHost/tests/test_neural_model_runner.cpp`
- Modify: `DaisyHost/CMakeLists.txt`

**Implement:**

- model weight loading
- deterministic inference
- stateful block wrapper suitable for both desktop host and Daisy firmware

### Task 18: Create a neural FX host app

**Files:**

- Create: `DaisyHost/include/daisyhost/apps/NeuralFxCore.h`
- Create: `DaisyHost/src/apps/NeuralFxCore.cpp`
- Modify: `DaisyHost/src/AppRegistry.cpp`
- Test: `DaisyHost/tests/test_neural_fx_core.cpp`

**Implement:**

- mono in
- mono or stereo out
- optional gain trim
- model load path
- menu pages for model and gain controls

### Task 19: Add a Daisy firmware adapter for the neural app

**Files:**

- Create: `patch/NeuralFx/NeuralFx.cpp`
- Create: `patch/NeuralFx/Makefile`
- Create: `patch/NeuralFx/README.md`

**Implement:**

- RTNeural runner on Daisy
- one very small exported model
- no dynamic allocation in the audio callback
- documented block size and memory assumptions

## Phase 7: Multi-Daisy Side Activity Requirements

This is a real side activity, but it is **not** a near-term implementation goal. It should be treated as a separate runtime and scheduling problem, not as an extension of the current single-board UI.

### Sweet spot conclusion

Do not begin multi-Daisy work until:

1. the canonical parameter contract is stable
2. single-node headless rendering is deterministic
3. app instancing is clean and repeatable

Only then is it worth building a host-side node graph or a VCV Rack adapter.

### What DaisyHost-native multi-Daisy actually requires

For multiple Patch, `patch.init()`, or future board instances inside DaisyHost or its VST3:

- multiple app instances with stable `nodeId`
- node-scoped parameter ids, control ids, and port ids
- per-node board-profile selection
- session or rack serialization for multiple nodes
- a graph scheduler for block processing
- typed cable routing across audio, CV, gate, and MIDI
- explicit feedback-loop and cycle policy
- a graph-aware headless render path
- node meters, selection, and debugging views

This is not “add another board widget to the editor.” It is a new runtime layer above the current single-node host.

### What a VCV Rack path actually requires

For a VCV Rack or Cardinal side path:

- a Rack adapter around the same shared app cores
- translation between Rack voltages or triggers and DaisyHost canonical port semantics
- translation between Rack sample-time processing and the app-core block model
- module widgets that decide how much Daisy board metaphor to retain
- state sync and preset serialization at the Rack layer

The shared core should stay the same. Only the adapter, scheduling, and module UI should differ.

### Recommended future order for this side activity

If pursued later:

1. add instance-safe app construction and node-scoped ids to DaisyHost
2. add a small internal graph runtime without UI-first complexity
3. add graph serialization and headless graph rendering
4. then decide whether DaisyHost-native rack UI or VCV Rack adapter is the better next surface

## Verification Matrix

At the end of each phase, run the smallest relevant checks.

### Host checks

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

### Firmware checks

From the specific firmware directory:

```sh
make
```

### Manual checks

For each new app:

- app appears in selector
- menu pages make sense
- MIDI input behaves as documented
- standalone launches
- VST3 can be loaded manually in a DAW before declaring complete

For training-readiness gates:

- deterministic state reset works
- direct parameter APIs work without GUI navigation
- effective-state readback reflects the real used values
- headless render produces reproducible output for the same input and seed

## Recommended Execution Order

Implement in this order:

1. Phase 1 canonical parameter contract and deterministic core
2. Phase 2 generalized host/app registry and selective host-parameter bridge
3. Phase 3 headless renderer and dataset-generation plumbing
4. Phase 4 SuperKnob and training-ready MetaController
5. Phase 5 Markov app and Daisy adapter
6. Phase 6 neural training, inference wrapper, host app, and Daisy adapter
7. Phase 7 multi-Daisy side activity only after the rest is stable

Reason:

- MetaController training does not make sense until the engine is deterministic, directly automatable, and headless-renderable.
- The renderer and dataset path should exist before serious training work.
- SuperKnob and MetaController features validate the shared parameter contract before the neural path adds extra complexity.
- Multi-node graphing is real future architecture, but it should not destabilize the single-node path that the book functionality depends on.
