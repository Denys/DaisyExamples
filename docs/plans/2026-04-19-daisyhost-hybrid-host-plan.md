# DaisyHost Hybrid Host Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Turn `DaisyHost` into a faithful, automatable single-node Daisy twin first, then use existing hosts for multi-instance playback and scripted evaluation before deciding whether a native `DaisyHost` patchbay is worth building.

**Architecture:** Keep `HostedAppCore` as the source of truth. Export one strong single-node `DaisyHost` plugin and standalone app, then let external hosts handle multi-instance routing while `DawDreamer` handles deterministic offline evaluation. Build a native multi-node graph host only if the external-host phase exposes concrete Daisy-specific gaps.

**Tech Stack:** C++17, JUCE, GoogleTest, CMake, Python, DawDreamer, libDaisy, DaisySP, optional Carla/Bespoke for manual multi-instance validation.

---

## Scope

This plan covers four deliverables:

1. a training-ready and playback-ready single-node `DaisyHost`
2. a repeatable offline evaluation harness
3. a verified external-host workflow for multiple Daisy instances
4. a decision gate for a native `DaisyHost` patchbay

Out of scope for this plan:

- full hardware emulation
- plugin-inside-plugin hosting as the first multi-instance solution
- a VCV Rack module rewrite
- neural-model training details beyond render/evaluation support

## Decision Rule

Do **not** build a native multi-node `DaisyHost` graph host until all of these are true:

- single-node `DaisyHost` exposes canonical host-visible parameters
- deterministic reset and state restore work
- headless rendering works
- multiple `DaisyHost` instances have been tested in at least one external host
- there is a documented gap that external hosts cannot solve cleanly

If those conditions are not met, continue investing in the single-node host plus external orchestration.

## Task 1: Harden The Single-Node Contract

**Files:**
- Modify: `DaisyHost/include/daisyhost/HostedAppCore.h`
- Modify: `DaisyHost/include/daisyhost/apps/MultiDelayCore.h`
- Modify: `DaisyHost/src/apps/MultiDelayCore.cpp`
- Modify: `DaisyHost/include/daisyhost/apps/TorusCore.h`
- Modify: `DaisyHost/src/apps/TorusCore.cpp`
- Test: `DaisyHost/tests/test_multidelay_core.cpp`
- Test: `DaisyHost/tests/test_torus_core.cpp`
- Test: `DaisyHost/tests/test_parameter_parity.cpp`

**Step 1: Write the failing tests**

Add tests that prove:

- parameters can be enumerated by stable id
- direct parameter set/get does not require menu navigation
- effective values can be read after smoothing or clamping
- state reset and restore are deterministic for `MultiDelay` and `Torus`

```cpp
TEST(MultiDelayCoreContract, RestoresStateDeterministically)
{
    MultiDelayCore core;
    core.Prepare(48000.0, 64);
    core.SetParameterValue("mix", 0.75f);
    const auto saved = core.SaveState();

    core.ResetToDefaultState();
    core.LoadState(saved);

    EXPECT_FLOAT_EQ(core.GetParameterValue("mix"), 0.75f);
    EXPECT_FLOAT_EQ(core.GetEffectiveParameterValue("mix"), 0.75f);
}
```

**Step 2: Run tests to verify they fail**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "multidelay|torus|parameter_parity"
```

Expected: failures for missing direct-parameter and state APIs.

**Step 3: Implement the minimal shared-core contract**

Add the minimal API surface needed by both hosted apps:

- `SetParameterValue(id, normalizedValue)`
- `GetParameterValue(id)`
- `GetEffectiveParameterValue(id)`
- `SaveState()`
- `LoadState(state)`
- `ResetToDefaultState()`

**Step 4: Run tests to verify they pass**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "multidelay|torus|parameter_parity"
```

Expected: target tests pass for both apps.

**Step 5: Commit**

```sh
git add DaisyHost/include/daisyhost/HostedAppCore.h DaisyHost/include/daisyhost/apps/MultiDelayCore.h DaisyHost/src/apps/MultiDelayCore.cpp DaisyHost/include/daisyhost/apps/TorusCore.h DaisyHost/src/apps/TorusCore.cpp DaisyHost/tests/test_multidelay_core.cpp DaisyHost/tests/test_torus_core.cpp DaisyHost/tests/test_parameter_parity.cpp
git commit -m "feat: harden hosted app parameter contract"
```

## Task 2: Add The Host Parameter Bridge

**Files:**
- Modify: `DaisyHost/include/daisyhost/AppRegistry.h`
- Modify: `DaisyHost/src/AppRegistry.cpp`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.h`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Create: `DaisyHost/tests/test_host_parameter_bridge.cpp`

**Step 1: Write the failing tests**

Add tests that prove:

- a selected subset of canonical parameters is exposed to the host
- app switching preserves stable parameter ids for shared semantic controls
- host save/load restores app id and bridged parameter values

```cpp
TEST(HostParameterBridge, RestoresSelectedAppAndParameters)
{
    DaisyHostPatchAudioProcessor processor;
    processor.SetHostedAppForTest("multidelay");
    processor.SetHostedParameterForTest("mix", 0.25f);

    const auto state = processor.CopyStateForTest();

    DaisyHostPatchAudioProcessor restored;
    restored.RestoreStateForTest(state);

    EXPECT_EQ(restored.GetHostedAppIdForTest(), "multidelay");
    EXPECT_FLOAT_EQ(restored.GetHostedParameterForTest("mix"), 0.25f);
}
```

**Step 2: Run tests to verify they fail**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "host_parameter_bridge|app_registry"
```

Expected: failures for missing host-visible bridge logic.

**Step 3: Implement the minimal bridge**

Expose only canonical parameters that meet all of these rules:

- stable semantic id
- meaningful in both standalone and VST usage
- safe to automate from a DAW or script

Do not expose menu cursor state, encoder motion, or editor-only toggles as host parameters.

**Step 4: Run tests to verify they pass**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "host_parameter_bridge|app_registry"
```

Expected: host bridge tests pass and plugin targets still build.

**Step 5: Commit**

```sh
git add DaisyHost/include/daisyhost/AppRegistry.h DaisyHost/src/AppRegistry.cpp DaisyHost/src/juce/DaisyHostPluginProcessor.h DaisyHost/src/juce/DaisyHostPluginProcessor.cpp DaisyHost/tests/test_host_parameter_bridge.cpp
git commit -m "feat: expose canonical host parameters"
```

## Task 3: Add A Headless Render And Evaluation Harness

**Files:**
- Modify: `DaisyHost/include/daisyhost/RenderRuntime.h`
- Modify: `DaisyHost/src/RenderRuntime.cpp`
- Create: `DaisyHost/tools/render_app.cpp`
- Create: `DaisyHost/training/render_dataset.py`
- Create: `DaisyHost/training/eval_scenarios.py`
- Create: `DaisyHost/tests/test_render_reproducibility.cpp`
- Modify: `DaisyHost/CMakeLists.txt`

**Step 1: Write the failing tests**

Add tests that prove:

- the same app, state, input, and seed render the same output repeatedly
- renders can be driven without GUI startup
- `MultiDelay` and `Torus` both render from the same runtime entry point

```cpp
TEST(RenderRuntime, RepeatsIdenticalOfflineRender)
{
    const auto first = RenderForTest("multidelay", "fixtures/multidelay_state.json");
    const auto second = RenderForTest("multidelay", "fixtures/multidelay_state.json");
    EXPECT_EQ(first.hash, second.hash);
}
```

**Step 2: Run tests to verify they fail**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "render_runtime|render_reproducibility"
```

Expected: failures for missing headless entry point or reproducibility support.

**Step 3: Implement the minimal renderer**

Support this flow:

- choose app id
- load saved state
- choose input source or WAV input
- set seed
- render WAV plus JSON metadata

**Step 4: Run tests and a smoke render**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests render_app
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "render_runtime|render_reproducibility"
```

Then run a smoke render:

```sh
DaisyHost/build/Release/render_app.exe --app multidelay --seconds 2 --input impulse --output DaisyHost/build/render-smoke/multidelay.wav
```

Expected: deterministic output plus metadata file.

**Step 5: Commit**

```sh
git add DaisyHost/include/daisyhost/RenderRuntime.h DaisyHost/src/RenderRuntime.cpp DaisyHost/tools/render_app.cpp DaisyHost/training/render_dataset.py DaisyHost/training/eval_scenarios.py DaisyHost/tests/test_render_reproducibility.cpp DaisyHost/CMakeLists.txt
git commit -m "feat: add headless render and evaluation harness"
```

## Task 4: Validate External Multi-Instance Hosting

**Files:**
- Create: `DaisyHost/training/multi_instance_eval.py`
- Create: `DaisyHost/docs/external-host-workflows.md`
- Create: `DaisyHost/tests/test_host_session_state_multiapp.cpp`
- Modify: `DaisyHost/tests/test_host_session_state.cpp`

**Step 1: Write the failing tests**

Add tests and scripted checks that prove:

- multiple app states can be instantiated independently
- app identity and parameter state do not bleed across instances
- scripted evaluation can route multiple instances in one run

```python
def test_two_instances_keep_independent_state():
    drum = load_plugin("DaisyHost Patch.vst3")
    bass = load_plugin("DaisyHost Patch.vst3")
    drum.set_state({"app_id": "multidelay"})
    bass.set_state({"app_id": "torus"})
    assert drum.get_state()["app_id"] == "multidelay"
    assert bass.get_state()["app_id"] == "torus"
```

**Step 2: Run tests to verify they fail**

Run:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3
ctest --test-dir DaisyHost/build -C Release --output-on-failure -R "host_session_state"
py -3 DaisyHost/training/multi_instance_eval.py --smoke
```

Expected: failures or missing-script errors until the harness exists.

**Step 3: Implement the minimal external-host workflow**

Use the script and doc to validate these scenarios:

- two `DaisyHost` instances in one offline evaluation run
- one instance as clocked or rhythmic source
- one instance as bass/lead or effect target
- one modulation lane driven by parameter automation, not editor gestures

Treat the external host phase as a verification layer, not a replacement for the internal app contract.

**Step 4: Run the validation**

Run:

```sh
py -3 DaisyHost/training/multi_instance_eval.py --scenario drum-bass
py -3 DaisyHost/training/multi_instance_eval.py --scenario modulated-duo
```

Expected: both scenarios complete and emit result metadata.

**Step 5: Commit**

```sh
git add DaisyHost/training/multi_instance_eval.py DaisyHost/docs/external-host-workflows.md DaisyHost/tests/test_host_session_state_multiapp.cpp DaisyHost/tests/test_host_session_state.cpp
git commit -m "test: validate external multi-instance DaisyHost workflows"
```

## Task 5: Add The Native Patchbay Decision Gate

**Files:**
- Create: `docs/plans/2026-04-19-daisyhost-patchbay-decision.md`
- Create: `DaisyHost/docs/patchbay-gap-analysis.md`

**Step 1: Write the decision template**

Document these evaluation questions:

- what concrete task failed in external hosts
- whether the failure was Daisy-specific or just missing documentation
- whether a standalone `DaisyHost` patchbay solves it with less complexity than continuing with DAW or scripted orchestration

**Step 2: Run the review**

Review evidence from:

- host-parameter automation tests
- headless render harness
- multi-instance evaluation scenarios
- manual playing tests in at least one external host

**Step 3: Decide**

Only approve a native patchbay if at least one of these is true:

- Daisy-specific CV/gate/audio/MIDI semantics are not representable cleanly
- per-node board-style UI and display semantics matter during graph play
- external hosts block required automation or evaluation workflows

If none are true, do not build the patchbay yet.

**Step 4: Commit**

```sh
git add docs/plans/2026-04-19-daisyhost-patchbay-decision.md DaisyHost/docs/patchbay-gap-analysis.md
git commit -m "docs: add DaisyHost patchbay decision gate"
```

## Verification Gate

At the end of each task, run the smallest meaningful full check:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

When shared Daisy firmware adapters change, also rebuild the affected targets:

```sh
make
```

from `patch/MultiDelay/` and `patch/Torus/`.

## Expected Outcome

If Tasks 1 through 4 are complete, you will already have:

- one faithful single-node Daisy plugin/app
- deterministic offline evaluation
- repeatable multi-instance experiments
- a clear answer on whether a native patchbay is actually needed

That is the point where MetaController, SuperKnob, Markov, and Neural FX work can proceed without building on unstable host foundations.
