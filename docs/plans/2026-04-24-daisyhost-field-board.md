# DaisyHost Field Board-Support Package Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a selectable `daisy_field` board-support shell through the existing DaisyHost board factory seam while leaving the current two-node Patch rack frozen.

**Architecture:** This is a board-support package, not a rack/runtime expansion. The package adds Field board metadata, Hub selection, safe board-id persistence, render/session metadata, and a passive Field panel shell. It must not widen rack topology, node routing, hosted-app bindings, or selected-node control semantics in the same package.

**Tech Stack:** C++17, JUCE, CMake, GoogleTest, DaisyHost `BoardProfile` / Hub / session / render contracts, libDaisy `DaisyField` hardware mapping, Python smoke harness.

---

## Controlling Decision

Next safe move:

> Treat the rack as frozen and start Daisy Field as a separate board-support package through the existing board factory seam.

That statement changes the implementation boundary from the earlier draft. Field support should start now as a narrow package around board identity and board presentation. It should not wait for, alter, or extend the visible rack implementation beyond consuming the already-landed board factory seam.

## Current Verification

Field is implementable in DaisyHost as a board-support shell.

Evidence:

- `include/daisyhost/BoardProfile.h` already defines a board profile abstraction and board factory entrypoints.
- `src/BoardProfile.cpp` currently constructs only `daisy_patch`; `tests/test_board_profile.cpp` explicitly expects `daisy_field` to be rejected.
- `src/HubSupport.cpp` currently registers only `daisy_patch`, so the Hub rejects `daisy_field`.
- `HostSessionState`, render scenarios, manifests, and effective snapshots already carry `boardId`.
- `src/juce/DaisyHostPluginProcessor.cpp` already refreshes `boardProfile_` from `boardId_` and selected node id.
- Local `../libDaisy/src/daisy_field.h` and `../field/README.md` define the Field surface as 8 knobs, 4 CV inputs, 2 CV outputs, 16 key controls with LEDs, 2 switches, gate in/out, TRS MIDI in/out, stereo audio I/O, and OLED display.

Current constraint:

- The current Release aggregate host gate remains affected by the known machine-level unit-test payload read/lock issue in this checkout.
- That blocker should not prevent a narrow board-support package from starting, but it does prevent claiming a final fully green Release gate until the environment is fixed.

## Package Boundary

In scope:

- `daisy_field` board id and display name
- Field `BoardProfile` shape, passive panel controls, decorations, and virtual ports
- Hub board registration and launch-plan propagation of `--board daisy_field`
- safe board-id handling in processor startup/session paths
- render scenario and manifest preservation or validation for `daisy_field`
- standalone smoke parameterization so Field shell startup can be smoke-tested
- docs and tracker closeout for exactly the shell support that landed

Out of scope for this package:

- live rack topology changes
- new rack presets, routes, route validation, or node-count changes
- mixed-board racks
- changes to `LiveRackTopology`
- changes to hosted-app DSP behavior
- widening `HostedAppPatchBindings`
- replacing fixed Patch live-control arrays with variable-width Field runtime arrays
- mapping Field K1-K8, A/B keys, SW1/SW2, or LEDs into app parameters as live runtime controls
- CV-output automation or cross-node CV routing

Explicitly deferred follow-on:

- a `DaisyHost Field Native Controls` package can later add real Field control interaction, app mappings, key/LED state, CV outputs, and Field-specific render events. That package needs its own plan because it touches processor/editor/runtime contracts, not just board support.

## Milestone 0: Claim The Board-Support Package And Run Preflight Checks

**Files:**
- Modify: `DaisyHost/PROJECT_TRACKER.md`
- Modify if skill evidence changes: `DaisyHost/SKILL_PLAYBOOK.md`

**Step 1: Confirm the working boundary**

Read:

```powershell
Get-Content README.md
Get-Content CHECKPOINT.md
Get-Content PROJECT_TRACKER.md
Get-Content SKILL_PLAYBOOK.md
Get-Content CHANGELOG.md
Get-Content include\daisyhost\BoardProfile.h
Get-Content src\BoardProfile.cpp
Get-Content src\HubSupport.cpp
Get-Content src\juce\DaisyHostPluginProcessor.h
Get-Content src\juce\DaisyHostPluginProcessor.cpp
```

Expected: Field is still absent, board factory seam exists, and rack/runtime work remains out of scope.

**Step 2: Claim the workstream**

Append a `PROJECT_TRACKER.md` row before code edits:

- workstream: `Daisy Field board-support shell`
- files/slice: `BoardProfile`, `HubSupport`, board-id-safe processor/session/render paths, smoke harness, docs
- blocker: full Release aggregate gate may remain environment-blocked; package will use targeted Debug/unit and Release smoke evidence until that is cleared

**Step 3: Run a non-invasive baseline check**

Run:

```powershell
git status --short --branch
cmake --build build --config Debug --target unit_tests
```

Expected: Debug unit target builds. If Debug build cannot start because of unrelated workspace drift, record the blocker before editing.

**Step 4: Commit**

```bash
git add DaisyHost/PROJECT_TRACKER.md DaisyHost/SKILL_PLAYBOOK.md
git commit -m "docs: claim Daisy Field board-support package"
```

## Milestone 1: Pin The Frozen-Rack Field Contract With Failing Tests

**Files:**
- Modify: `DaisyHost/tests/test_board_profile.cpp`
- Modify: `DaisyHost/tests/test_hub_support.cpp`
- Modify: `DaisyHost/tests/test_host_session_state.cpp`
- Modify: `DaisyHost/tests/test_render_runtime.cpp`
- Optional modify: `DaisyHost/tests/test_effective_host_state_snapshot.cpp`

**Step 1: Add board-profile expectations**

Add a failing test that asserts:

- `GetSupportedBoardIds()` contains `daisy_field`
- `TryCreateBoardProfile("daisy_field", "nodeA")` succeeds
- `boardId == "daisy_field"`
- `nodeId == "nodeA"`
- `displayName == "Daisy Field"`
- display is `128x64`
- there are 8 knob controls, 16 key controls, 2 switch/button controls
- there are 2 audio inputs, 2 audio outputs, 4 CV inputs, 2 CV outputs, 1 gate input, 1 gate output, and 2 MIDI ports
- every Field `panelBounds` rectangle stays inside normalized panel coordinates

**Step 2: Preserve unknown-board rejection**

Change the existing `daisy_field` rejection assertion to `unknown_board`.

**Step 3: Add Hub expectations**

Add a failing test that asserts:

- `GetBoardRegistrations()` includes `{"daisy_field", "Daisy Field", true}`
- `NormalizeHubProfile(...)` preserves valid `daisy_field`
- `BuildHubLaunchPlan(...)` accepts `selection.boardId = "daisy_field"`
- generated Play/Test arguments include `--board daisy_field`

**Step 4: Add board-id persistence and render expectations**

Add failing tests that assert:

- `HostSessionState` round-trips `board daisy_field`
- legacy sessions still default to `daisy_patch`
- render scenario parsing preserves `"boardId": "daisy_field"`
- render manifests preserve `boardId == "daisy_field"`
- unknown render `boardId` fails with a clear board-related error once validation is added

**Step 5: Run the red tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|HubSupportTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"
```

Expected: tests fail because `daisy_field` is not registered or constructible. Failures should not involve `LiveRackTopology` or hosted-app behavior.

**Step 6: Commit**

```bash
git add DaisyHost/tests/test_board_profile.cpp DaisyHost/tests/test_hub_support.cpp DaisyHost/tests/test_host_session_state.cpp DaisyHost/tests/test_render_runtime.cpp DaisyHost/tests/test_effective_host_state_snapshot.cpp
git commit -m "test: specify Daisy Field board-support contract"
```

## Milestone 2: Add The Field Board Profile And Hub Registration

**Files:**
- Modify: `DaisyHost/include/daisyhost/BoardProfile.h`
- Modify: `DaisyHost/src/BoardProfile.cpp`
- Modify: `DaisyHost/src/HubSupport.cpp`
- Modify: `DaisyHost/tests/test_board_profile.cpp`
- Modify: `DaisyHost/tests/test_hub_support.cpp`

**Step 1: Extend board metadata narrowly**

Add:

```cpp
BoardProfile MakeDaisyFieldProfile(const std::string& nodeId = "node0");
```

If needed, add only generic control kinds required for Field panel metadata:

```cpp
enum class ControlKind
{
    kKnob,
    kEncoder,
    kButton,
    kKey,
    kSwitch,
};
```

Do not add runtime behavior to these enums in this milestone.

**Step 2: Implement `MakeDaisyFieldProfile()`**

Create the profile with:

- board id: `daisy_field`
- display name: `Daisy Field`
- 8 passive knob controls labeled `K1` through `K8`
- 16 passive key controls labeled `A1..A8` and `B1..B8`
- 2 passive switch controls labeled `SW1` and `SW2`
- OLED display at `128x64`
- stereo audio input/output ports
- 4 CV input ports
- 2 CV output ports
- gate input and gate output ports
- MIDI input and MIDI output ports
- Field panel text and decoration metadata

Use node-scoped ids:

```text
nodeA/control/field_knob_1
nodeA/control/field_key_a_1
nodeA/control/field_sw_1
nodeA/port/field_cv_in_1
nodeA/port/field_cv_out_1
nodeA/port/field_gate_in_1
nodeA/port/field_gate_out_1
```

**Step 3: Register the board**

Update:

- `GetSupportedBoardIds()`
- `TryCreateBoardProfile(...)`
- `kBoardRegistrations` in `src/HubSupport.cpp`

Do not change `GetDefaultBoardId()`. It must remain `daisy_patch`.

**Step 4: Run targeted tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|HubSupportTest)"
```

Expected: Field profile and Hub tests pass. Patch board tests still pass unchanged.

**Step 5: Commit**

```bash
git add DaisyHost/include/daisyhost/BoardProfile.h DaisyHost/src/BoardProfile.cpp DaisyHost/src/HubSupport.cpp DaisyHost/tests/test_board_profile.cpp DaisyHost/tests/test_hub_support.cpp
git commit -m "feat: add Daisy Field board-support profile"
```

## Milestone 3: Make Board Selection Safe Without Touching Rack Topology

**Files:**
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.cpp`
- Modify: `DaisyHost/src/juce/DaisyHostPluginProcessor.h`
- Modify: `DaisyHost/src/HostSessionState.cpp`
- Modify: `DaisyHost/src/RenderRuntime.cpp`
- Modify: `DaisyHost/tests/test_host_session_state.cpp`
- Modify: `DaisyHost/tests/test_render_runtime.cpp`
- Optional modify: `DaisyHost/tests/test_effective_host_state_snapshot.cpp`

**Step 1: Add safe board-id application**

Add a helper such as:

```cpp
bool TryApplyBoardId(const std::string& requestedBoardId);
```

Behavior:

- empty board id means default `daisy_patch`
- known board id updates `boardId_` and refreshes `boardProfile_`
- unknown board id falls back to `daisy_patch` or returns false without throwing from plugin startup/session restore paths
- Hub startup request and session restore use the same helper

**Step 2: Keep rack state unchanged**

Do not edit:

- `LiveRackTopology`
- rack node count
- route expansion/inference
- selected-node targeting behavior
- audio routing

Field board selection should be board metadata and panel/profile selection only in this package.

**Step 3: Validate render board ids**

Render should:

- default missing board id to `daisy_patch`
- accept `daisy_patch`
- accept `daisy_field`
- reject unknown board ids with a clear error
- continue to write the selected `boardId` to manifest output

**Step 4: Run targeted tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"
```

Expected: board-id selection, session, render, and snapshot tests pass with both `daisy_patch` and `daisy_field`. No rack-topology tests should need edits.

**Step 5: Commit**

```bash
git add DaisyHost/src/juce/DaisyHostPluginProcessor.cpp DaisyHost/src/juce/DaisyHostPluginProcessor.h DaisyHost/src/HostSessionState.cpp DaisyHost/src/RenderRuntime.cpp DaisyHost/tests/test_host_session_state.cpp DaisyHost/tests/test_render_runtime.cpp DaisyHost/tests/test_effective_host_state_snapshot.cpp
git commit -m "feat: make DaisyHost board selection safe"
```

## Milestone 4: Render A Passive Field Panel Shell

**Files:**
- Modify: `DaisyHost/src/juce/DaisyHostPluginEditor.cpp`
- Modify: `DaisyHost/src/juce/DaisyHostPluginEditor.h`
- Optional modify: `DaisyHost/tests/test_board_profile.cpp`

**Step 1: Make panel drawing profile-driven**

The editor currently assumes Patch-specific surface ids such as:

```text
node0/surface/ctrl1_mix
node0/surface/ctrl2_primary
node0/surface/ctrl3_secondary
node0/surface/ctrl4_feedback
node0/surface/enc1
```

Refine the editor so:

- Patch keeps its current interactive control layout and appearance
- Field can draw profile decorations, texts, display, controls, and ports without relying on Patch surface ids
- Field knobs, keys, switches, LEDs, and CV outputs are passive visual elements only
- missing Patch-specific surface ids do not leave stale controls visible over the Field panel

**Step 2: Keep Patch behavior stable**

Do not change:

- existing Patch control labels
- existing Patch slider count
- existing rack header
- topology selector
- selected-node behavior
- Patch app parameter mapping

**Step 3: Run targeted build**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
cmake --build build --config Release --target DaisyHostPatch_Standalone
```

Expected: unit target and standalone target build. If Release unit execution remains blocked, record that separately; do not treat it as a Field-panel regression.

**Step 4: Commit**

```bash
git add DaisyHost/src/juce/DaisyHostPluginEditor.cpp DaisyHost/src/juce/DaisyHostPluginEditor.h DaisyHost/tests/test_board_profile.cpp
git commit -m "feat: render passive Daisy Field board shell"
```

## Milestone 5: Add Field Startup And Render Smoke Coverage

**Files:**
- Modify: `DaisyHost/tests/run_smoke.py`
- Add: `DaisyHost/training/examples/field_cloudseed_shell_smoke.json`
- Modify: `DaisyHost/tests/test_render_runtime.cpp`
- Optional modify: `DaisyHost/CMakeLists.txt`

**Step 1: Parameterize standalone smoke board selection**

Extend `tests/run_smoke.py` so standalone smoke can launch:

```powershell
py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --board daisy_field --app cloudseed
```

Expected: the real standalone process starts, consumes `--board daisy_field`, survives the warmup window, and exits cleanly.

**Step 2: Add a Field shell render scenario**

Add:

```json
{
  "boardId": "daisy_field",
  "appId": "cloudseed",
  "renderConfig": {
    "sampleRate": 48000,
    "blockSize": 48,
    "durationSeconds": 1.0,
    "outputChannelCount": 2
  },
  "audioInput": {
    "mode": "sine",
    "level": 2.0,
    "frequencyHz": 220.0
  },
  "timeline": []
}
```

Do not add Field-specific control events in this package. Those belong to the deferred Field-native controls package.

**Step 3: Extend render smoke**

Include the Field shell scenario in render smoke and assert the output manifest records:

```json
"boardId": "daisy_field"
```

**Step 4: Run smoke checks**

Run:

```powershell
cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone
py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --board daisy_field --app cloudseed
py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120
```

Expected: standalone Field shell smoke passes; render smoke passes for existing Patch scenarios plus the Field shell scenario.

**Step 5: Commit**

```bash
git add DaisyHost/tests/run_smoke.py DaisyHost/training/examples/field_cloudseed_shell_smoke.json DaisyHost/tests/test_render_runtime.cpp DaisyHost/CMakeLists.txt
git commit -m "test: add Daisy Field shell smoke coverage"
```

## Milestone 6: Full Gate And Documentation Closeout

**Files:**
- Modify: `DaisyHost/README.md`
- Modify: `DaisyHost/CHECKPOINT.md`
- Modify: `DaisyHost/PROJECT_TRACKER.md`
- Modify: `DaisyHost/SKILL_PLAYBOOK.md`
- Modify: `DaisyHost/CHANGELOG.md`

**Step 1: Run the strongest available gate**

Run:

```powershell
cmd /c build_host.cmd
```

Expected: full Release host gate passes if the machine-level unit-test payload lock is cleared.

If the known payload lock remains, also run and record the strongest targeted fallback:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|HubSupportTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"
cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone
py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --board daisy_field --app cloudseed
py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120
```

Expected: targeted Debug tests and Release smoke pass.

**Step 2: Update docs with exact scope**

Use precise wording:

- `Field board-support shell` if only this plan lands
- `Field-native controls` only after a future package maps real K1-K8, keys, switches, LEDs, and CV outputs into runtime behavior

Update:

- `README.md`: board selection now includes `daisy_field`; Field is a passive shell in this package
- `CHECKPOINT.md`: dated verification and remaining manual gaps
- `PROJECT_TRACKER.md`: files touched, exact checks run, blocker state
- `SKILL_PLAYBOOK.md`: skill evidence if materially used
- `CHANGELOG.md`: user-facing shell support bullet

**Step 3: Commit**

```bash
git add DaisyHost/README.md DaisyHost/CHECKPOINT.md DaisyHost/PROJECT_TRACKER.md DaisyHost/SKILL_PLAYBOOK.md DaisyHost/CHANGELOG.md
git commit -m "docs: document Daisy Field board-support shell"
```

## Final Verification Gate

Required before calling the board-support package complete:

```powershell
cmd /c build_host.cmd
```

If that remains blocked by the known environment issue, completion must be phrased as:

```text
Field board-support shell is targeted-test-backed and smoke-backed; full Release aggregate gate remains blocked by the known machine-level unit-test payload lock.
```

Required manual checks before user-facing release:

- Hub shows `Daisy Field`
- Hub Play/Test launches standalone with `--board daisy_field`
- standalone renders the Field shell without layout corruption
- Field passive controls do not overlap or resize badly
- Patch board remains default and visually intact
- Patch rack topology controls still behave as before
- Field render manifest records `boardId == "daisy_field"`
- DAW-side VST3 load still works for the default Patch path

Completion wording must be one of:

- `Field board-support shell supported`
- `Field board-support shell build-verified only - manual DAW/GUI validation not performed`
- `Field board-support shell targeted-test-backed and smoke-backed - full Release aggregate gate blocked by environment`

Do not use:

- `Field-native controls supported`
- `Field runtime supported`
- `mixed-board rack supported`
- `Field rack supported`

until a separate Field-native controls/runtime package lands and is verified.
