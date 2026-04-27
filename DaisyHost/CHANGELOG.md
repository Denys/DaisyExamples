# Changelog

Canonical change tracker for `DaisyHost`.

## [Unreleased]

- require manager-readable WP explanations in DaisyHost planning and closeout:
  - future WP plans, workstream table updates, and completion handoffs must
    state what is being implemented or was done, why it matters, what remains,
    and what is explicitly out of scope
- add the repo-local next-WP recommendation workflow:
  - add `tools/suggest_next_wp.py` to read `WORKSTREAM_TRACKER.md` and print
    the next recommended work package, runner-up, overlap risk, explicit waits,
    and first safe implementation slice after WP closeout
  - wire the recommender into CTest as `DaisyHostNextWpSuggester`
  - update handoff workflow docs so completed WPs record the recommender result
    in `PROJECT_TRACKER.md`
  - verify with direct Python tests, the registered CTest, and full
    `cmd /c build_host.cmd` passing Release `ctest` `233/233`
- add the first WS10 external debug-surface slice:
  - existing `DaisyHostCLI snapshot --json` and `render --json` outputs now
    include additive `debugState` readback for board id, selected node,
    entry/output role labels, routes, selected-node target cues, and render
    timeline target counts
  - keep CLI syntax, routing presets, route validation, scenario/session
    schemas, firmware, hardware, and DAW/VST3 validation scope unchanged
  - verify with red/green `CliPayloadsTest`, direct Debug CLI snapshot/render
    checks, a broader Debug contract subset, and full `cmd /c build_host.cmd`
    passing Release `ctest` `232/232`
- add the initial TF11 node-targeted event/readback slice:
  - render manifests now report resolved `targetNodeId` values for inferred
    parameter, CV/gate, menu, surface-control, and selected-node MIDI timeline
    events without changing routing presets or route validation semantics
  - CLI render-result JSON now includes render `nodes`, `routes`, and
    `executedTimeline` debug readback alongside the existing summary fields
  - verify with targeted TF11 Debug tests, broader Debug render/CLI/session/
    snapshot/topology coverage, full Debug `ctest`, and full
    `cmd /c build_host.cmd` passing Release `ctest` `227/227`, then the later
    TF10 hardening gate passing `230/230`
- complete the TF10 live-rack routing-contract foundation:
  - add shared `LiveRackRoutePlan` construction with resolved audio endpoints,
    deterministic processing order, and route-plan validation for the existing
    four frozen topology presets
  - route offline render and live processor audio chaining through the shared
    route plan while keeping scenario/session shapes and user-facing presets
    unchanged
  - harden the contract with validation-only, accepted-config, failed-plan
    preservation, and durable unsupported-shape wording tests
  - verify with targeted Debug topology, render-runtime, session, and snapshot
    checks plus full `cmd /c build_host.cmd` passing Release `ctest`
    `230/230`
  - keep new route presets, graph editing, larger rack graphs, and non-audio
    route semantics deferred to explicit `WS9` scope
- productionize the existing two-node rack UX for WS8:
  - keep the four existing audio-only topology presets and add clearer
    operator-facing topology labels/direction copy
  - replace ambiguous node role text with selected-node-aware role labels for
    audio entry, audio output, combined entry/output, and inactive routing
  - add compact selected-node/per-node context and board-aware Patch/Field copy
    explaining that live controls, drawer pages, CV/gate, Field K/A/B/SW
    controls, and modulation edits target the selected rack node
  - verify with red/green rack helper coverage, targeted Debug rack/session/
    snapshot/board/render tests, focused Field/RSP status tests, Release render
    smoke, and the full `cmd /c build_host.cmd` gate passing `216/216`
- add DaisyHost v1 host-side modulation lanes:
  - destinations are continuous hosted-app parameters with native min/max,
    unit, precision, and discrete-target exclusion metadata
  - each destination supports up to four fixed lanes sourced from `CV 1-4` or
    `LFO 1-4`, evaluated as native-unit sum plus clamp over the stored base
    parameter
  - hosted apps now expose transient effective-parameter override hooks so
    modulation changes DSP/readback without overwriting knob, menu,
    automation, or session base values
  - session persistence moves to `HostSessionState` v6 with `modlane` records
    while v1-v5 sessions continue to load with no modulation lanes
  - effective-state snapshots and CLI JSON now report selected modulation
    destination, lane config, live source value, native contribution,
    base/result native values, normalized result, and clamped state
  - Daisy Field drawer pages are renamed `Play`, `Mod`, and `Rack`; `Mod`
    provides destination selection plus compact source/depth/bypass/clear lane
    rows
- fix Daisy Field A/B key buttons becoming disabled for `subharmoniq`:
  - Field key UI enablement now follows the Field key binding availability
    contract instead of requiring a MIDI note number
  - keeps generic apps clickable through MIDI-note bindings while allowing
    `subharmoniq` A/B keys to remain clickable as sequencer/rhythm/transport
    menu actions
- harden TF12 verification and CLI adoption docs:
  - record the 2026-04-26 modulation-lane verification truth as the then-current
    `211/211` `build_host.cmd` gate
  - document the DaisyHostCLI agent/CI smoke sequence using existing commands
    without adding new CLI surface
  - keep older `159/159`, `168/168`, `196/196`, `197/197`, and `202/202`
    counts as dated historical evidence only
- add native `DaisyHostCLI.exe` for agent/CI workflows:
  - discovery commands for hosted apps, board profiles, and test input modes
  - app/board description JSON for scenario generation and control inspection
  - scenario validation, offline render, effective-state snapshot, smoke, and
    doctor commands built on existing DaisyHost runtime contracts
  - CLI smoke entries wired into CTest and `build_host.ps1`
- add `field/DaisyHostController` as a controller-only Daisy Field firmware
  target for DaisyHost:
  - sends standard USB MIDI using libDaisy `MidiUsbHandler`, with no audio
    callback or custom DaisyHost protocol
  - maps K1-K8 to CC 20-27, CV1-CV4 to CC 28-31, A1-B8 to notes 60-75, and
    SW1/SW2 to momentary CC 80/81 on MIDI channel 1
  - includes firmware `README.md`, `CONTROLS.md`, OLED/LED feedback, and a
    manual DaisyHost MIDI learn validation checklist
  - builds with `make`, passes QAE validation, and flashes/verifies through
    ST-Link on 2026-04-26; USB MIDI enumeration and manual DaisyHost
    validation remain pending
- separate Daisy Field hosted-app navigation from right-side drawer navigation:
  - SW1/X and SW2/C now rotate the active app menu instead of changing RSP
    pages
  - RSP Page 1/2/3 remains mouse/debug controlled by its page buttons
  - RSP Page 1 adds latched CV target dropdowns for knob-controlled
    parameters, with target selection switching that CV source to Generator
  - CV target dropdowns now show both default and alternative Field knob target
    sets as `Kx.1` and `Kx.2`, while preserving unsafe target filtering and
    avoiding overlap between default and alternative K mappings
  - the later oversized Page 1 modulation-bay/keyboard-helper experiment was
    reverted; Page 1 remains the compact split layout with Keyboard MIDI and
    MIDI tracker on the left and CV generator/manual CV/Gate controls on the
    right
  - CloudSeed adds Space, Motion, Arp, and Advanced app pages, with Advanced
    Field knobs limited to safe EQ/seed parameters that avoid disabling,
    muting, bypassing, or disconnecting audio input/output
- add a compact CloudSeed arpeggiator as a deterministic parameter-stepper:
  - new canonical `arp_enabled`, `arp_rate`, `arp_pattern`, `arp_depth`, and
    `arp_target` state in `DaisyCloudSeedCore`
  - effective-state modulation over CloudSeed performance groups while stored
    knob/menu parameter values remain unchanged
  - hosted `cloudseed` `Arp` menu section plus status/info display surfacing
  - render-runtime and `cloudseed_smoke.json` coverage for the arp path
  - deliberate tradeoff: this is not MIDI-note arpeggiation; CloudSeed remains
    an audio-input effect
- add `subharmoniq` as a first-class supported hosted app and Field firmware
  target with:
  - a portable `DaisySubharmoniqCore` inspired by Subharmonicon-style
    subharmonic oscillators, dual 4-step sequencers, integer rhythm dividers,
    quantization, and Round 1 SVF-style low-pass filtering
  - a hosted `apps::SubharmoniqCore` wrapper registered as app id
    `subharmoniq`
  - `field/SubharmoniqField` firmware with K1-K8 page controls, A/B key
    sequencer/rhythm/transport actions, SW1/SW2 pseudo-encoder menu
    navigation, CV/Gate/MIDI mappings, OLED, LEDs, and CV OUT sequencer
    monitors
  - an internal tempo clock fix so `B7` play can produce rhythm-triggered
    audio without requiring external MIDI clock or Gate In first
  - playable-default tuning for the internal-clock path: real attack/decay
    envelope phase handling, stronger output/filter/source defaults, and
    pickup-style Field knob startup so physical K1-K8 positions do not mute
    the patch before a knob is moved
  - DaisyHost Field UI refinements for SW1/SW2 navigation, X/C virtual switch
    shortcuts, Field knob parameter labels, octave-aligned keyboard view,
    CV/Gate trace cues, and top-grouped audio/gate/MIDI artwork
  - host tests, Field firmware `make`, QAE validation, and ST-Link flashing
    passing on 2026-04-26; manual hardware validation remains pending
  - follow-up Field key-row correction so physical `B7`/`B8`, not physical
    `A7`/`A8`, trigger play/reset on hardware, plus DaisyHost
    `subharmoniq` Field keys now route to dedicated sequencer/rhythm/transport
    actions instead of generic chromatic MIDI notes
- add `polyosc` as a first-class supported hosted app with:
  - a portable `DaisyPolyOscCore` wrapper for `patch/PolyOsc`
  - Patch K1-K3 oscillator-frequency controls, K4 global-frequency offset,
    encoder waveform selection, individual oscillator outputs on outs 1-3, and
    mixed output on out 4 as the host stereo source
  - host-side Field support through existing board mappings, with K1-K4
    mirroring Patch controls and K5 mapped to `waveform`
  - `training/examples/polyosc_smoke.json` and
    `training/examples/field_polyosc_surface_smoke.json` render smoke coverage
- add HW/App adapter pipeline v0:
  - `tools/generate_field_adapter.py` generates Daisy Field firmware adapter
    projects from checked-in shared-core JSON specs
  - `tools/adapter_specs/field_multidelay.json` captures the first golden
    `MultiDelayCore` -> Daisy Field adapter contract
  - `tools/audit_firmware_portability.py` classifies firmware projects as
    `portable-core-ready`, `needs-core-extraction`, or `not-supported-by-v0`
  - generated `field/MultiDelayGenerated` builds and passes QAE validation;
    generated-adapter flashing remains manual/optional
- add `field/MultiDelay` as the first Daisy Field firmware adapter:
  - uses `daisy::DaisyField` with the shared
    `DaisyHost/src/apps/MultiDelayCore.cpp`
  - maps K1-K4 to the current MultiDelay Patch-page controls, K5 to
    `delay_tertiary`, CV1 to the tertiary CV path, CV OUT 1 to K5 as
    `0..5V`, CV OUT 2 to `0V`, SW1 to `Fire Impulse`, and SW2 to OLED status
    page switching
  - includes Field firmware `README.md` and `CONTROLS.md`
  - builds with `make`, passes QAE validation, and flashes/verifies through
    ST-Link on 2026-04-25; full manual audio/control/CV hardware validation
    remains pending
- fix a board/app-independent knob-drag crash by serializing hosted-app core
  access between audio processing and message-thread control/UI refresh paths
- polish Field K5-K8 mapping so ranked extra Field knobs exclude explicit
  K1-K4 parameter targets reported by hosted apps before falling back to the
  older control-id conversion path
- make Field surface controls carry profile target ids and have the editor use
  board-profile lookup for Field knobs, keys, and switches instead of relying
  on Field-only surface-id construction
- add `tests/run_smoke.py` as a direct-entrypoint smoke harness for
  `DaisyHost Patch.exe` and `DaisyHostRender.exe`
- assign the DaisyHost Patch compact icon asset to `DaisyHost Hub.exe` so the
  launcher carries the same Windows executable icon family as the standalone
  Patch host
- wire `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke` into CTest so the
  normal host gate now covers standalone startup stability plus render-output
  smoke and `multidelay` checksum determinism
- add a fixed five-slot DAW automation bridge with stable ids
  `daisyhost.slot1` .. `daisyhost.slot5`, mapped onto the active app's
  top-ranked automatable canonical parameters
- add processor-side effective-state snapshot/readback for canonical
  parameters, mapped automation slots, CV state, gate state, and current
  audio-input configuration
- extend the effective-state snapshot with `selectedNodeId`, `nodeCount`, and
  active-node MetaController readback
- expand the effective-state snapshot again for the visible rack with `boardId`,
  `entryNodeId`, `outputNodeId`, `routes`, and lightweight `nodeSummaries`
- add named MetaControllers for `multidelay` and `cloudseed`, surfaced through
  the existing mirrored menu/drawer path and derived from canonical parameters
- promote `cloudseed` from a Workstream-6 pilot to a first-class supported
  hosted app
- add `HostSessionState` v4 with `node` and `route` records while keeping
  legacy single-node session text backward-compatible
- extend render scenario and manifest contracts with optional `nodes[]` /
  `routes[]` plus per-node summaries
- add a hidden two-node audio-chain proof path to the render runtime and reject
  unsupported cross-node CV, gate, and MIDI routes with clear errors
- add `DaisyCloudSeedCore` as a portable shared wrapper around the imported
  `third_party/CloudSeedCore` with canonical performance/expert state,
  deterministic seed handling, bypass/clear/randomize utilities, and effective
  raw-parameter readback
- add `cloudseed` as the first Workstream-6 hosted-app pilot with:
  - stereo host input/output on Patch channels 1/2
  - `Space` / `Motion` performance pages that remap the four Patch knobs
  - encoder/menu sections for `Pages`, `Program`, `Utilities`, and `Info`
  - utility actions for `Bypass`, `Clear Tails`, `Randomize Seeds`, and
    `Interpolation`
  - fixed DAW automation priority for `mix`, `size`, `decay`, `diffusion`, and
    `pre_delay`
- refresh the JUCE processor's active patch bindings during core snapshot
  updates so page-based control remaps change live DaisyHost knob labels and
  control ids instead of staying cached from app creation
- add `training/examples/cloudseed_smoke.json`, extend render/runtime coverage
  for `cloudseed`, and include the new scenario in `DaisyHostRenderSmoke`
- add `DaisyBraidsCore` as a portable shared wrapper around vendored Braids
  oscillator code with a curated percussion-first model subset, `24`-sample
  inner rendering, deterministic trigger/state handling, and canonical
  parameter capture/restore
- add `braids` as a first-class supported hosted app with:
  - MIDI-first triggering plus optional `gate_in_1` strike alias
  - `Drum` / `Finish` performance pages that remap the four Patch knobs
  - encoder/menu sections for `Pages`, `Model`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Randomize Model`, and `Panic`
  - fixed DAW automation priority for `model`, `tune`, `timbre`, `color`, and
    `signature`
- add `training/examples/braids_smoke.json`, extend render/runtime smoke
  coverage for `braids`, and include the new scenario in
  `DaisyHostRenderSmoke`
- add `DaisyHarmoniqsCore` as a portable shared additive wrapper with
  deterministic harmonic-state capture/restore, MIDI/gate triggering, and
  `Spectrum` / `Envelope` page bindings
- add `harmoniqs` as a first-class supported hosted app with:
  - MIDI-first triggering plus optional `gate_in_1` retrigger alias
  - `Spectrum` / `Envelope` knob-page remapping
  - encoder/menu sections for `Pages`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Init Spectrum`, `Randomize Spectrum`, and
    `Panic`
- add `DaisyVASynthCore` as a portable shared seven-voice subtractive wrapper
  with deterministic polyphonic state, MIDI/gate triggering, and `Osc` /
  `Filter` / `Motion` page bindings
- add `vasynth` as a first-class supported hosted app with:
  - seven-voice MIDI-first note handling plus optional `gate_in_1` trigger
    alias
  - `Osc` / `Filter` knob-page remapping surfaced through the current
    DaisyHost menu flow, with `Motion` kept in the shared canonical model
  - encoder/menu sections for `Pages`, `Oscillators`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Init Patch`, `Stereo Sim`, and `Panic`
- add `training/examples/harmoniqs_smoke.json` and
  `training/examples/vasynth_smoke.json`, extend render/runtime smoke coverage
  for both apps, and include the new scenarios in `DaisyHostRenderSmoke`
- add `training/examples/field_cloudseed_shell_smoke.json`, extend standalone
  smoke to launch `--board daisy_field`, and include the Field board-support
  shell scenario in `DaisyHostRenderSmoke`
- add DaisyHost-local `include/stmlib/...` compatibility shims so the vendored
  Braids sources build under the Windows MSVC host toolchain without modifying
  the shared repo `stmlib/` copy
- apply a local MSVC portability fix to
  `third_party/CloudSeedCore/DSP/RandomBuffer.cpp` by replacing the upstream
  VLA scratch buffers with `std::vector`
- add helper-layer unit coverage for the automation bridge and effective-state
  snapshot contract, then extend the rerun host gate to `79/79` on 2026-04-22
- add helper-layer and contract coverage for MetaControllers plus rack-ready
  session/render state, then rerun the full host gate to `89/89` on 2026-04-23
- add `LiveRackTopology` as the shared helper for the four visible audio-only
  rack presets, reverse inference, and legal-route validation
- add the first visible WS7 live rack MVP:
  - exactly two live hosted nodes (`node0`, `node1`)
  - selected-node-targeted Patch controls, drawer/menu state, test inputs,
    keyboard MIDI, and five-slot DAW automation
  - editor rack header with per-node app selectors, selected-node highlight,
    role labels, and topology selector
  - operator-facing audio-only topology presets:
    `node0_only`, `node1_only`, `node0_to_node1`, `node1_to_node0`
- add a board factory seam so the runtime creates board profiles by `boardId`
  instead of hardcoding Patch construction in the processor path
- add `daisy_field` as a Field board-support shell through the existing board
  factory seam, with passive profile metadata for eight knobs, four CV inputs,
  two CV outputs, gate in/out, stereo audio I/O, OLED, keys, switches, and LED
  affordances
- register `daisy_field` in Hub, standalone startup, session, and render board
  handling while preserving `daisy_patch` as the default board and keeping the
  rack topology/runtime semantics frozen
- add `BoardControlMapping` for host-side Field native controls:
  - K1-K4 mirror the selected node's current Patch page bindings
  - K5-K8 map to the next four automatable selected-node parameters by
    `importanceRank`, leaving missing targets disabled
  - CV1-CV4 and Gate In reuse the existing host CV/gate input paths
  - A1-B8 emit 16 chromatic MIDI notes from the selected node's current
    keyboard octave
- render interactive Field K1-K8 controls and A1-B8 momentary keys on the
  `daisy_field` panel while preserving Patch control behavior for
  `daisy_patch`
- add `surface_control_set` timeline events for Field render scenarios and
  validate unknown Field surface control ids with clear errors
- add `training/examples/field_vasynth_native_controls_smoke.json`, extend
  render smoke coverage for Field native controls, and check final mapped
  parameter evidence in the smoke manifest
- add Field extended host surface support for host-side Field
  outputs/switches/LEDs:
  - CV OUT 1-2 are derived monitor outputs for the K5/K6 mapped parameters,
    reported as normalized values and `0..5V` evidence
  - SW1/SW2 are momentary selected-app utility triggers
  - key LEDs, switch LEDs, Gate In LED, and Gate Out LED are derived,
    non-persisted indicators
- extend the effective-state snapshot and render manifest with Field surface
  state so automated tests can prove CV OUT, switch, and LED behavior
- render SW1/SW2, CV OUT indicators, and LED indicators on the `daisy_field`
  panel while preserving `daisy_patch` behavior and the frozen rack topology
- add `training/examples/field_extended_surface_smoke.json`, extend render
  smoke coverage for Field extended host surface support, and validate final
  Field CV OUT, SW, and LED manifest evidence
- add `training/examples/field_node_target_surface_smoke.json`, extend render
  smoke coverage for selected-node Field surface evidence, and make multi-node
  Field manifests report the selected node's Field surface state
- change Hub `Play / Test` launch planning to write the supported
  `hub_launch_request.json` startup request for selected board/app launches,
  while direct standalone `--board` / `--app` arguments remain supported
- make the selected-node mirror copy board-aware so `daisy_field` no longer
  describes Field runtime controls as Patch controls
- move session persistence to `HostSessionState` v5 with rack globals
  `boardId`, `selectedNodeId`, `entryNodeId`, and `outputNodeId`
- extend render timeline events with `targetNodeId` for multi-node non-ID-
  scoped events and prove both forward and reverse two-node serial audio chains
  directly through `DaisyHostRender.exe`
- change unit-test registration from runtime discovery to source-scanned
  `gtest_add_tests`, keep the target name `unit_tests`, and launch unit cases
  through `tests/run_unit_test_payload.py`
- change the Release unit-test payload path again so wrapper-driven configure
  runs emit a fresh per-run payload under
  `build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
- make `build_host.ps1` PATH normalization accept shells that expose only
  `Path` as well as shells that expose the duplicate `PATH` key
- make `build_host.ps1` also inject a fresh
  `DAISYHOST_UNIT_TEST_RUN_TAG` on each configure so wrapper-driven reruns do
  not collide with stale quarantined Release payload paths in this checkout
- fix the Braids render-runtime unit scenario duration so its final gate event
  is inside the render window
- add `build_host.ps1` plus `build_host.cmd` as the standard local host-gate
  entrypoint, encapsulating the shell-specific `Path` / `PATH` normalization
  needed by MSBuild-backed commands in this checkout
- widen the standalone smoke process-query timeout so slower Windows
  process-path discovery does not cause a false timeout on a healthy launch
- rerun the full wrapper-driven host gate on 2026-04-24 and pass `136/136`
- rerun the full wrapper-driven host gate on 2026-04-25 and pass `144/144`
- rerun the full wrapper-driven host gate on 2026-04-25 after Field extended
  host surface support and pass `152/152`
- rerun the full wrapper-driven host gate on 2026-04-25 after Field refinement
  closeout and pass `155/155`
- rerun the full wrapper-driven host gate on 2026-04-25 after Field ergonomics
  polish and board-generic UI cleanup and pass `159/159`
- route Windows VST3 manifest generation through
  `tools/write_vst3_manifest.ps1` so `juce_vst3_helper.exe` is invoked through
  PowerShell instead of the failing direct MSBuild / `cmd` launch path
- document the smoke harness commands and update local verification docs to
  reflect automated standalone/render smoke coverage
- document that external DAW/VST3 load validation is intentionally deferred
  until a post-Workstream-7 manual pass
- promote `PROJECT_TRACKER.md` to a first-class DaisyHost tracking document and
  per-iteration testing ledger
- require mandatory per-iteration testing evidence for docs and code work, with
  targeted test expectations before handoff and the full host gate before
  milestone or merge handoff
- define the concurrent-thread protocol: `AGENTS.md` stays authoritative for
  ownership slices and contracts, while `PROJECT_TRACKER.md` stays authoritative
  for active work order, testing entries, and handoffs
- add `ROADMAP.d2` plus a compiled `ROADMAP.svg` for the current governance,
  workstream, and parallelization roadmap
- add `WORKSTREAM_TRACKER.md` as the dedicated post-WS7 portfolio tracker and
  mirror the same next-workstream plan into `PROJECT_TRACKER.md`
- add `SKILL_PLAYBOOK.md` as the dedicated DaisyHost file for skill-related
  activities and split skill ratings into `Expected UF` vs evidence-based
  `Observed UF`
- relabel stale verification counts and artifact assertions in local docs so
  unrerun runtime claims stay dated instead of implied-current

## [0.2.0] - 2026-04-18

Implemented refresh:

- promote `DaisyHost/` to a pinned first-party workspace in repo guidance
- add DaisyHost-local agent docs, checkpointing, and a canonical changelog flow
- remap Patch controls to:
  - `CTRL 1` = dry/wet
  - `CTRL 2` = primary delay
  - `CTRL 3` = secondary delay
  - `CTRL 4` = feedback
  - `CV 1` = tertiary delay
  - `ENC 1` = menu navigation and edit/confirm
- make every DSP parameter editable from the OLED menu with `last touch wins`
- redesign the host UI toward a more clearly Patch-like vector layout
- add `Saw` as a test input and restore `Host In` as the standalone default
- surface version/build identity and recent change history through the app mirror/About flow
- add standalone icon assets, wire the compact icon into `ICON_SMALL`, and apply a runtime standalone window icon
- hide the JUCE standalone yellow mute banner while keeping live input muted by default
- generalize DaisyHost into a multi-app Patch host with an app registry/factory layer
- persist selected hosted app in session state
- keep `MultiDelay` as the default deterministic fixture
- add `Torus` as the first nontrivial second hosted app with:
  - DaisyHost-native canonical parameter and menu model
  - dynamic knob-assignment menu
  - stereo output on Patch outputs 1/2
  - gate-triggered strum and audio-input exciter semantics
- rebind the processor and editor to active app metadata and patch bindings instead of hardcoded `MultiDelay` ids
- add Phase 3 headless render support with:
  - `DaisyHostRender` CLI target
  - scenario JSON loading and validation
  - deterministic offline `WAV + JSON manifest` output
  - timeline support for parameter, CV, gate, MIDI, audio-input, impulse, and
    compatibility menu events
- add `training/render_dataset.py`, dataset job examples, and render workflow
  docs under `training/`
- add `DaisyHost Hub` as a small GUI launcher for board/app/activity selection
- add hub-local profile persistence separate from plugin/session state
- add a startup-request handoff so `Play / Test` opens the standalone host with
  the selected app

## [0.1.0] - Existing Workspace Baseline

Observed baseline already present in the repo before the `0.2.0` refresh lands:

- `DaisyHost` builds a `VST3` plugin and standalone app around the extracted `patch/MultiDelay` core
- current built artifacts exist under:
  - `build/DaisyHostPatch_artefacts/Release/VST3/DaisyHost Patch.vst3`
  - `build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe`
- host-side test target is `unit_tests`
- host UI already includes mouse-accessible controls, a virtual keyboard, computer-keyboard MIDI input, CC learn support, and a MIDI tracker
- the current source version remains `0.1.0` until the main integrator slice updates `CMakeLists.txt`
