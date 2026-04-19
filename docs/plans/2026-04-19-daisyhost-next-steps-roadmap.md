# DaisyHost Next-Steps Roadmap — 2026-04-19

## Goal

Advance `DaisyHost` as the primary platform first, then layer in the useful
book-inspired features on top of a deterministic, automatable, reusable host.

This roadmap assumes:

- Phase 1 is done: canonical parameter contract, deterministic reset/state, and
  parity-oriented tests exist.
- Phase 2 is done: multi-app host support exists with `multidelay` and `torus`.
- The next work should strengthen `DaisyHost` itself before pursuing larger AI
  or meta-controller features.

## Strategy

Use a **DaisyHost-first** sequence:

1. finish the missing platform plumbing
2. add deterministic offline rendering and dataset support
3. add semantic control layers like MetaController
4. then add book-style feature families such as Markov and Neural FX

Why:

- MetaController training is premature without headless rendering and stable
  state automation.
- Neural work is fragile without reproducible render + export paths.
- A stronger host platform will make future board expansion and possible
  multi-Daisy graph work much easier.

## Current State

Already in place:

- deterministic shared app-core contract
- session-persisted app selection
- multi-app host with `MultiDelay` and `Torus`
- Daisy Patch-like UI shell
- host-side CV/audio debug generators
- Daisy firmware references for `patch/MultiDelay` and `patch/Torus`

Still missing at the platform level:

- selective JUCE/VST host parameter bridge for canonical parameters
- headless render tool
- dataset-generation flow
- explicit effective-state inspection API in host automation paths
- broader app registry and reusable app metadata beyond current demos

## Roadmap

### Phase 3: Automation Bridge And Host Hardening

**Purpose:** make `DaisyHost` externally automatable and less editor-driven.

**Deliverables**

- selective JUCE/VST parameter exposure for canonical parameters
- tighter app metadata and capability reporting
- app selection and parameter automation survive save/load reliably
- effective-parameter readback visible to the host layer
- tests for host-parameter mapping and app switching

**Key files**

- `DaisyHost/include/daisyhost/HostedAppCore.h`
- `DaisyHost/include/daisyhost/AppRegistry.h`
- `DaisyHost/src/AppRegistry.cpp`
- `DaisyHost/src/juce/DaisyHostPluginProcessor.h`
- `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- `DaisyHost/tests/test_app_registry.cpp`
- `DaisyHost/tests/test_host_parameter_bridge.cpp`

**Done when**

- DAW-visible parameters exist for a selected subset of canonical app params
- switching between `multidelay` and `torus` does not break host automation
- app metadata is clean enough that future apps can be dropped in without
  processor/editor rewrites

### Phase 4: Headless Rendering And Dataset Generation

**Purpose:** make training and reproducible experimentation possible.

**Deliverables**

- CLI render tool for hosted apps
- deterministic WAV rendering from app id + state + input config
- saved metadata for app id, seed, parameter values, and input source settings
- small dataset sweep scripts for batch rendering

**Key files**

- `DaisyHost/tools/render_app.cpp`
- `DaisyHost/training/render_dataset.py`
- `DaisyHost/training/configs/*.yaml`
- `DaisyHost/tests/test_render_app_smoke.cpp`
- `DaisyHost/tests/test_render_reproducibility.cpp`

**Done when**

- the same app/state/input/seed produces the same output repeatedly
- `multidelay` and `torus` can both render headlessly
- dataset scripts can produce a small batch without GUI interaction

### Phase 5: MetaController / SuperKnob Layer

**Purpose:** implement the useful semantic-control part of the book.

**Deliverables**

- reusable `MacroController`
- `MetaControllerModel` that maps semantic inputs to internal parameters
- one small SuperKnob-style hosted app or extension demo
- training-ready interface that targets canonical parameters, not UI gestures

**Key files**

- `DaisyHost/include/daisyhost/MacroController.h`
- `DaisyHost/src/MacroController.cpp`
- `DaisyHost/include/daisyhost/MetaControllerModel.h`
- `DaisyHost/src/MetaControllerModel.cpp`
- `DaisyHost/include/daisyhost/apps/SuperKnobDemoCore.h`
- `DaisyHost/src/apps/SuperKnobDemoCore.cpp`
- `DaisyHost/tests/test_macro_controller.cpp`
- `DaisyHost/tests/test_metacontroller_model.cpp`

**Done when**

- one semantic control can drive multiple internal parameters deterministically
- the mapping can be exercised from the GUI, direct API, and headless render
- training experiments can target macro outputs without replaying menu actions

### Phase 6: Markov / Improviser Path

**Purpose:** bring in the most portable Part 3 book ideas.

**Deliverables**

- shared-core `MarkovModel`
- hosted `MarkovImproviserCore`
- MIDI learn/capture/generate flow inside DaisyHost
- Patch firmware adapter for a Markov-based app

**Key files**

- `DaisyHost/include/daisyhost/improviser/MarkovModel.h`
- `DaisyHost/src/improviser/MarkovModel.cpp`
- `DaisyHost/include/daisyhost/apps/MarkovImproviserCore.h`
- `DaisyHost/src/apps/MarkovImproviserCore.cpp`
- `patch/MarkovImproviser/*`

**Done when**

- MIDI can be captured, modeled, and regenerated deterministically
- the host app works in standalone and VST form
- the Daisy firmware adapter builds and follows the shared-core semantics

### Phase 7: Neural FX Path

**Purpose:** add the smallest realistic neural-audio workflow.

**Deliverables**

- Python training/export scripts
- RTNeural-compatible model export
- shared-core neural inference wrapper
- one hosted `NeuralFxCore`
- one tiny Daisy firmware adapter proving deployability

**Key files**

- `DaisyHost/training/train_lstm.py`
- `DaisyHost/training/export_rtneural.py`
- `DaisyHost/include/daisyhost/neural/NeuralModelRunner.h`
- `DaisyHost/src/neural/NeuralModelRunner.cpp`
- `DaisyHost/include/daisyhost/apps/NeuralFxCore.h`
- `DaisyHost/src/apps/NeuralFxCore.cpp`
- `patch/NeuralFx/*`

**Done when**

- a small trained model can run in DaisyHost
- the same exported model can run in a constrained Daisy firmware target
- model inference remains deterministic and realtime-safe

## Side Activity: Multi-Daisy / Rack Direction

This is explicitly **not** the next main phase.

It only becomes practical after:

- automation bridge is stable
- headless rendering exists
- app instances are fully clean and node-scoped

Only then should work begin on:

- multi-instance graph runtime
- node-scoped port and parameter ids
- cable routing for audio/CV/gate/MIDI
- graph serialization
- optional DaisyHost-native rack UI or VCV Rack adapter

## Validation Rules

At the end of every phase, prefer the smallest meaningful full check:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

When shared cores or firmware adapters change, also rebuild:

```sh
make
```

from `patch/MultiDelay/` and `patch/Torus/`, plus any new firmware target
added in later phases.

Manual checks remain required for:

- standalone launch stability
- DAW-side VST3 load
- app switching
- parameter automation sanity
- MIDI behavior
- any new renderer/training outputs

## Priority Order

If only one next step is chosen now, do this:

1. **Phase 3: Automation Bridge And Host Hardening**

If two phases are planned:

1. **Phase 3**
2. **Phase 4**

That is the minimum point where DaisyHost becomes a strong foundation for the
book-inspired MetaController and neural workflows.
