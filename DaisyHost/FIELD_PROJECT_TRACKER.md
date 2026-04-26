# DaisyHost Field Project Tracker

Last updated: 2026-04-26

Use this file as the Field-specific tracker for DaisyHost. It keeps
`daisy_field` host implementation work, validation work, and deferred hardware
scope separate from the broader DaisyHost workstream portfolio.

## Current Field Status

Manager explanation: Daisy Field is already implemented as an automatically
tested host-side DaisyHost board surface, Sprint F3 added both the first
hardware-facing firmware adapter and the first semi-automatic adapter pipeline,
the 2026-04-26 SubharmoniqField pass adds the first new Field-first
instrument built around a new portable DaisyHost core, and the
2026-04-26 DaisyHostController pass adds a standard USB MIDI controller
firmware path for controlling DaisyHost from real Field hardware.
The hand-written `field/MultiDelay` adapter proves real Field firmware can reuse
the shared `MultiDelayCore`; the generated `field/MultiDelayGenerated` adapter
proves that the same pattern can now be produced from a checked-in spec. The
hand-written MultiDelay adapter and SubharmoniqField are ST-Link
flash-verified, while the generated adapter remains build/QAE-verified only
until an explicit flash pass is requested and run.

- `daisy_patch` remains the default board.
- `daisy_field` is supported through the existing board factory seam.
- Host-side Field support includes:
  - Field board-support shell metadata
  - K1-K8 host controls
  - CV1-CV4 host inputs
  - Gate In host input
  - A1-B8 MIDI key buttons
  - CV OUT 1-2 host-side monitor outputs
  - SW1/SW2 host-side momentary utility triggers
  - key, switch, Gate In, and Gate Out LED evidence
  - Hub startup-request launch planning
  - selected-node render/smoke evidence
  - Field K5-K8 ranked extras that exclude the real K1-K4 parameter targets
  - board-profile target metadata used by the Field editor surface lookup
- First Field firmware adapter:
  - `field/MultiDelay` builds against `DaisyField` and the shared
    `DaisyHost/src/apps/MultiDelayCore.cpp`
  - ST-Link flash/verify succeeded on 2026-04-25
  - manual Field audio/control/CV checklist remains pending until performed on
    the connected hardware
- First adapter pipeline:
  - `tools/generate_field_adapter.py` emits a Daisy Field firmware project from
    `tools/adapter_specs/field_multidelay.json`
  - `field/MultiDelayGenerated` builds and passes QAE validation on 2026-04-25
  - `tools/audit_firmware_portability.py` classifies existing Field firmware as
    `portable-core-ready`, `needs-core-extraction`, or `not-supported-by-v0`
  - generated adapter flash and manual hardware validation remain pending
- New Field-first portable-core instrument:
  - `DaisyHost/DaisySubharmoniqCore` and `apps::SubharmoniqCore` implement
    the first-class hosted `subharmoniq` app
  - `field/SubharmoniqField` builds against the shared portable core and maps
    Field K1-K8, A/B keys, SW1/SW2 pseudo-encoder navigation, CV/Gate, MIDI,
    OLED, LEDs, and CV outputs to the Round 1 Subharmonicon-inspired model
  - 2026-04-26 audible-default follow-up changed the core from a barely
    nonzero one-sample trigger to a host-tested playable envelope/output path,
    and changed the Field adapter to pickup-style knob startup
  - firmware `make`, QAE validation, and ST-Link `make program` flash/verify
    passed on 2026-04-26
  - the manual Field hardware checklist remains pending
- DaisyHost USB MIDI controller firmware:
  - `field/DaisyHostController` is a controller-only Field firmware target;
    it sends K1-K8 as CC 20-27, CV1-CV4 as CC 28-31, A1-B8 as notes 60-75,
    and SW1/SW2 as momentary CC 80/81 on MIDI channel 1
  - the target uses libDaisy `MidiUsbHandler`, has no audio callback, and is
    intended to drive DaisyHost through its existing MIDI input, tracker, and
    CC learn paths rather than a custom protocol
  - firmware `make`, QAE validation, and ST-Link `make program` flash/verify
    passed on 2026-04-26
  - user manual check on 2026-04-26 confirmed knobs, buttons, and keys display
    correctly on the Field screen
  - USB MIDI enumeration, DaisyHost MIDI learn, and computer-side MIDI
    validation remain pending
- Latest full DaisyHost host gate from this checkout:
  - `cmd /c build_host.cmd`: passed on 2026-04-26
  - Release `ctest`: passed, `202/202`
  - DaisyHostCLI agent/CI adoption checks passed directly; older host-gate
    counts remain historical only.

## Separate Validation Todo

This item is intentionally outside the implementation sprint backlog.

### DaisyHostController Hardware / MIDI Validation

Status: To do

Goal:

- Flash `field/DaisyHostController` and prove real Daisy Field USB MIDI control
  reaches DaisyHost standalone and, later, the VST3 through a DAW.

Manual validation checklist:

- `make program` flashes successfully through ST-Link. Passed on 2026-04-26
  with OpenOCD `** Verified OK **`.
- Local Field OLED feedback shows K1-K8 knob moves, SW1/SW2 button presses,
  and A1-B8 key events. Passed by user report on 2026-04-26.
- The computer sees the Daisy Field as a USB MIDI device.
- DaisyHost standalone can enable the device in `Settings...`.
- DaisyHost MIDI tracker shows K1-K8 CC 20-27, CV1-CV4 CC 28-31, A1-B8 notes
  60-75, and SW1/SW2 momentary CC 80/81.
- DaisyHost MIDI learn can bind at least one Field knob to a hosted-app
  control and move that parameter.
- No stuck notes, repeated idle CC flooding, or OLED/LED stalls are observed.

### Field DAW / VST3 Validation

Manager explanation: this proves the already-implemented Field host surface in
the real plugin workflow. It should not be counted as new Field implementation
unless the validation finds a defect that requires code changes.

Status: To do

Goal:

- Load `DaisyHost Patch.vst3` in a DAW and prove `daisy_field` behaves as
  expected in a real plugin host.

Manual validation checklist:

- Open a DAW that can load local VST3 plugins.
- Load `DaisyHost Patch.vst3`.
- Select `daisy_field`.
- Confirm the Field panel renders instead of Patch-only panel wording.
- Confirm K1-K8 interact with the selected node.
- Confirm A1-B8 trigger MIDI notes for a MIDI-capable app.
- Confirm SW1/SW2 momentary utility behavior.
- Confirm host CV/Gate controls still affect the selected node.
- Save and reload a session with `boardId == "daisy_field"`.
- Record DAW name, DAW version, plugin path, date, pass/fail, and any defects.

Evidence required before marking done:

- dated manual validation notes
- screenshots or screen recording if available
- exact build/gate used before DAW validation
- defect tickets or tracker entries for any failures

Out of scope for this todo:

- Field firmware
- real hardware voltage output
- mixed-board racks
- new DAW automation banks
- arbitrary Field-specific app redesign

## Implementation Sprint Backlog

### Sprint F1: Field Ergonomics Polish

Status: Implemented on 2026-04-25

Manager explanation: this makes Field feel intentional to users instead of
merely technically supported. It improves the mapping labels and app-facing
control choices while keeping the rack frozen.

Goal:

- Improve Field-specific labels, OLED hints, and mapping choices for the
  current hosted apps without changing rack topology or board selection.

Implemented result:

- Hosted apps now report explicit Patch-page knob parameter ids where the
  current page is parameter-backed.
- Field K5-K8 still use ranked automatable selected-node parameters, but they
  now exclude those explicit K1-K4 parameter targets before falling back to the
  older control-id conversion path.
- Locked Field extras are covered for MultiDelay, Torus, CloudSeed, Braids,
  Harmoniqs, and VA Synth.

Manager explanation: this prevents Field's extra knobs from duplicating the
same parameter already shown on K1-K4, so the Field surface exposes more useful
controls without changing Patch behavior.

Likely files:

- `src/BoardControlMapping.cpp`
- `include/daisyhost/BoardControlMapping.h`
- `src/juce/DaisyHostPluginEditor.cpp`
- `src/juce/DaisyHostPluginProcessor.cpp`
- hosted app metadata under `src/apps/`
- `tests/test_board_control_mapping.cpp`
- `tests/test_render_runtime.cpp`
- `tests/run_smoke.py`
- `training/examples/*field*.json`
- `README.md`
- `CHECKPOINT.md`
- `PROJECT_TRACKER.md`
- `CHANGELOG.md`

Red tests first:

- Field K5-K8 labels remain useful for each supported app.
- Field OLED or panel evidence describes Field controls without Patch-only
  wording.
- Existing Patch mappings remain unchanged.
- Existing selected-node Field smoke scenarios still pass.

Implementation notes:

- Keep `daisy_patch` as default.
- Keep `LiveRackTopology` unchanged.
- Prefer app metadata and board-mapping helpers over ad hoc editor branches.
- Do not expand DAW automation in this sprint.

Verification:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|RenderRuntimeTest|HostSessionStateTest|LiveRackTopologyTest)"
cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone
py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120
py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120
cmd /c build_host.cmd
```

### Sprint F2: Board-Generic UI Cleanup

Status: Implemented on 2026-04-25

Manager explanation: this reduces future board-support cost. Field should not
need special-case UI code wherever the board profile already knows what to
draw.

Goal:

- Remove remaining Patch-shaped assumptions from the editor and board rendering
  path where the active board is `daisy_field`.

Implemented result:

- Field profile surface controls now expose target ids for knobs, keys, and
  switches.
- The Field editor layout and interactivity checks can resolve controls through
  board-profile metadata instead of constructing Field-only surface ids.
- Patch profile/control hierarchy tests remain unchanged.

Manager explanation: this makes the current Field GUI less brittle and gives
future boards a clearer path through metadata instead of hardcoded naming rules.

Likely files:

- `include/daisyhost/BoardProfile.h`
- `src/BoardProfile.cpp`
- `src/juce/DaisyHostPluginEditor.h`
- `src/juce/DaisyHostPluginEditor.cpp`
- `tests/test_board_profile.cpp`
- any editor policy tests already covering layout safety
- `README.md`
- `CHECKPOINT.md`
- `PROJECT_TRACKER.md`
- `CHANGELOG.md`

Red tests first:

- Board profile metadata can drive Field-visible labels and affordances.
- Patch rendering remains stable.
- Field rendering does not expose Patch-only language for active surface
  controls.

Implementation notes:

- Keep executable/artifact names such as `DaisyHost Patch.exe` unchanged unless
  a separate branding decision is made.
- Avoid a redesign; this is cleanup and data-driven rendering.
- Do not create mixed-board rack behavior.

Verification:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|RenderRuntimeTest|LiveRackTopologyTest)"
py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120
cmd /c build_host.cmd
```

### Sprint F3: Field Hardware/Firmware Parity Plan And First Adapter

Status: Adapter build-verified and ST-Link flash-verified on 2026-04-25;
manual functional hardware checklist pending.

Manager explanation: this is the first hardware-facing sprint. It should start
only when the team is ready to distinguish host simulation from real Daisy Field
behavior with actual firmware build and hardware validation evidence.

Goal:

- Add the first real Field firmware adapter for one narrow app/control path.
- Use `field/MultiDelay` because `MultiDelayCore` is DaisyHost's regression
  fixture and already backs the Patch firmware adapter.
- Prove build and ST-Link flash now; require the manual hardware checklist
  before claiming full hardware validation.

Likely files:

- `../field/MultiDelay/Makefile`
- `../field/MultiDelay/MultiDelay.cpp`
- `../field/MultiDelay/README.md`
- `../field/MultiDelay/CONTROLS.md`
- `../DaisyHost/src/apps/MultiDelayCore.cpp` reused as a source dependency,
  with no host-runtime change required
- DaisyHost trackers/checkpoint/changelog for verification truth

Red tests / checks first:

- `make` from `field/MultiDelay` first failed as expected because
  `MultiDelay.cpp` did not exist yet:
  `No rule to make target 'build/MultiDelay.o'`.
- Control mapping checklist is captured in `field/MultiDelay/CONTROLS.md`.
- Host-side Field claims remain separate from firmware/hardware claims.

Implementation notes:

- This is not a DaisyHost-only sprint; it adds a firmware example under
  `field/MultiDelay`.
- The adapter uses `daisy::DaisyField` plus
  `daisyhost::apps::MultiDelayCore`.
- Audio uses Field input 1 and stereo outputs 1/2.
- K1-K4 map to the current MultiDelay Patch-page controls; K5 maps to
  `delay_tertiary`; K6-K8 are intentionally unused.
- CV1 mirrors the existing tertiary-delay CV input path.
- CV OUT 1 mirrors K5 as `0..5V`; CV OUT 2 is held at `0V`.
- SW1 triggers the existing `Fire Impulse` utility; SW2 toggles the OLED
  status page only and does not change DSP state.
- Hardware validation is required before claiming full real Field support.
- Do not change DaisyHost rack topology.

Verification:

```powershell
cd ..\field\MultiDelay
make
```

Result: passed on 2026-04-25 after switching the firmware adapter to `OPT = -Os`
to fit Daisy Field flash.

```powershell
cd ..\field\MultiDelay
$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .
```

Result: passed on 2026-04-25 with `0 error(s), 0 warning(s)`.

```powershell
cd ..\field\MultiDelay
make program
```

Result: passed on 2026-04-25. OpenOCD detected STLINK `V3J7M2`, target voltage
`3.271571`, programmed the STM32H7 target, and reported `** Verified OK **`.

```powershell
cd ..\..\DaisyHost
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(MultiDelayCoreTest|BoardControlMappingTest|RenderRuntimeTest)"
cmd /c build_host.cmd
```

Result: passed on 2026-04-25. Targeted Debug CTest passed `52/52`; the full
DaisyHost wrapper gate passed with Release `ctest` `159/159`.

Manual hardware checklist status: pending. Do not mark Sprint F3 fully
hardware-validated until audio input/output, K1-K5, K6-K8 no-op behavior, CV1,
CV OUT 1/2 voltage behavior, SW1, SW2, LEDs, OLED, and audio-dropout checks are
performed and recorded.

### Sprint F4: HW/App Adapter Pipeline v0

Status: Implemented on 2026-04-25 for the `MultiDelay` golden target; generated
adapter is build/QAE-verified, not flash-verified.

Manager explanation: this turns the F3 hand-written adapter into a repeatable
pattern. It does not translate arbitrary firmware. It generates Daisy Field
adapter glue around portable DaisyHost shared cores and gives existing firmware
projects a read-only portability audit.

Goal:

- Generate a Daisy Field firmware adapter from a checked-in DaisyHost app spec.
- Keep `MultiDelay` as the only v0 golden target.
- Audit existing firmware projects for shared-core readiness without rewriting
  them.

Implemented result:

- `tools/generate_field_adapter.py` reads a JSON spec and emits:
  - `Makefile`
  - `<App>.cpp`
  - `README.md`
  - `CONTROLS.md`
- `tools/adapter_specs/field_multidelay.json` captures the v0 MultiDelay Field
  adapter contract.
- `tools/audit_firmware_portability.py` reports:
  - `portable-core-ready`
  - `needs-core-extraction`
  - `not-supported-by-v0`
- `field/MultiDelayGenerated` was generated and built as proof of the pipeline.

Verification:

```powershell
py -3 -m pytest -q tests/test_field_adapter_generator.py -p no:cacheprovider
py -3 tools/generate_field_adapter.py --spec tools/adapter_specs/field_multidelay.json --out ..\field\MultiDelayGenerated
cd ..\field\MultiDelayGenerated
make
$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .
cd ..\..\DaisyHost
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(MultiDelayCoreTest|BoardControlMappingTest|RenderRuntimeTest)"
cmd /c build_host.cmd
```

Result: passed on 2026-04-25. Pytest passed `3/3`; generated firmware `make`
passed with FLASH `121376 B` / `92.60%`; QAE passed with `0 error(s), 0
warning(s)`; targeted Debug CTest passed `52/52`; full host wrapper gate passed
with Release `ctest` `159/159`.

Caveat: `make program` was not run for `field/MultiDelayGenerated` because the
plan marked generated-adapter flashing optional and firmware flashing is a
hardware side effect. Generated adapter status is build/QAE-verified only.

### Sprint F5: Field Automation Expansion Decision

Manager explanation: this is a DAW/product decision sprint, not a small mapping
cleanup. DaisyHost currently has a stable five-slot DAW bridge; expanding Field
automation could help production workflows but risks saved-session and DAW
compatibility if handled casually.

Goal:

- Decide whether Field needs more DAW-visible automation or better labels for
  the existing fixed slots.

Likely files:

- `include/daisyhost/HostAutomationBridge.h`
- `src/HostAutomationBridge.cpp`
- `src/juce/DaisyHostPluginProcessor.cpp`
- `tests/test_host_automation_bridge.cpp`
- `tests/test_host_session_state.cpp`
- DAW validation notes
- `README.md`
- `CHECKPOINT.md`
- `PROJECT_TRACKER.md`
- `CHANGELOG.md`

Red tests first:

- Saved automation ids remain backward compatible.
- Field board selection does not break existing five-slot Patch behavior.
- Any new label or slot behavior is deterministic across app changes.

Implementation notes:

- Prefer label improvements before adding new automation parameters.
- Do not break existing `daisyhost.slot1` through `daisyhost.slot5` ids.
- Require DAW validation if host-visible automation behavior changes.

Verification:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(HostAutomationBridgeTest|HostSessionStateTest|RenderRuntimeTest)"
cmd /c build_host.cmd
```

### Sprint F6: SubharmoniqField Hardware Validation And Filter Round 2

Status: Round 1 build/QAE/ST-Link flash-verified on 2026-04-26; no-audio root
causes fixed in the portable core and Field startup adapter; manual hardware
validation and Round 2 filter modes pending.

Manager explanation: SubharmoniqField is now a real firmware target and hosted
app, but it should not be described as hardware-validated until the hands-on
checklist passes. The current build now includes an internal tempo clock plus
louder envelope/output defaults, so `B7` play can
trigger audible host-tested envelopes without first receiving external MIDI
clock or Gate In pulses. Field K1-K8 also use pickup-style startup so low
physical knob positions do not immediately mute the patch.

Goal:

- Flash `field/SubharmoniqField` through ST-Link. Done on 2026-04-26 with
  OpenOCD `** Verified OK **`.
- Complete the manual audio/control/CV/MIDI checklist in
  `field/SubharmoniqField/CONTROLS.md`.
- Only after Round 1 hardware behavior is stable, implement the planned Filter
  page switch between `SVF LPF`, `SVF BPF`, and ladder-style `LPF`.

Verification:

```powershell
cd ..\field\SubharmoniqField
make
$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .
make program
```

Result: `make` passed as up to date, QAE passed with `0 error(s),
0 warning(s)`, and `make program` passed on 2026-04-26. OpenOCD detected
STLINK `V3J7M2`, target voltage `3.263618`, programmed
`build/SubharmoniqField.elf`, reported `** Verified OK **`, and reset the
target.

## Explicitly Out Of Scope Until Separately Approved

- mixed-board racks
- freeform routing graph editor
- Field firmware claims without firmware build and hardware evidence
- real CV output voltage claims from host-only smoke tests
- DAW/VST3 support claims without manual DAW validation evidence
- broad app redesigns that are not tied to a tested Field workflow

## Next Safe Starting Point

Start with the separate DAW / VST3 validation todo if the goal is release
confidence in the plugin path. Start with Sprint F6 if the goal is
SubharmoniqField hardware proof or the Round 2 filter-mode expansion. Start a
new Field ergonomics sprint only if it has a concrete tested workflow beyond
the F1/F2 host-side cleanup already landed.
