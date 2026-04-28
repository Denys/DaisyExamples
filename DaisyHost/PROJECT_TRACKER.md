# DaisyHost Project Tracker

Last updated: 2026-04-28

Use this file as the running DaisyHost status ledger. Update it after each
meaningful implementation or verification iteration so the active work order,
testing evidence, parallel-thread coordination, and next steps stay explicit.

## Manager-Language Rule

Future DaisyHost planning and closeout must lead in manager terms before file,
class, or test detail. Plans should state what needs to be implemented, why it
matters, what it unlocks, what depends on it, what is out of scope, and what
evidence will prove it is done. Closeouts should state what was implemented,
why it matters, what changed for users/agents/CI, what remains blocked or
unclaimed, what was explicitly out of scope, the verification evidence, and the
next safe starting point.

Latest fully green host gate from this checkout:

- `cmd /c build_host.cmd`: passed on 2026-04-28
- underlying aggregate:
  - `cmake -S . -B build`: passed
  - `cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
  - `ctest --test-dir build -C Release --output-on-failure`: passed, `269/269`
- smoke tests included:
  - `DaisyHostNextWpSuggester`
  - `DaisyHostStandaloneSmoke`
  - `DaisyHostRenderSmoke`
  - `DaisyHostCliListApps`
  - `DaisyHostCliDescribeApp`
  - `DaisyHostCliDescribeBoard`
  - `DaisyHostCliValidateScenario`
  - `DaisyHostCliDoctor`
  - `DaisyHostCliRender`

Latest implementation iteration:

- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: TF15 doctor source/build readiness expansion closeout
- Manager-readable result:
  - Done: existing `DaisyHostCLI doctor --json` now reports source/build,
    artifact, CTest-registration, and environment readiness while preserving
    the previous command and stable top-level JSON fields.
  - Why it matters: agents and CI can cheaply decide whether the checkout is
    ready for `gate`, smoke, or targeted CTest before spending time on a full
    build.
  - What it unlocks: missing source files, stale build trees, missing Hub/VST3
    artifacts, missing smoke registrations, unsupported configs, and duplicate
    Windows `Path` / `PATH` hazards are now visible in one structured payload.
  - What remains: `gate` is still the command that actually runs
    `build_host.cmd`; broader verification hardening remains `TF12` scope and
    adoption workflow proof remains `WS13` scope.
  - Out of scope: no new CLI command, no full-gate execution inside `doctor`,
    no GUI automation, live plugin control, DAW/VST3 validation, firmware
    flashing, product routing changes, or generic shell/git wrapping.
- Affected surfaces:
  - `include/daisyhost/DoctorDiagnostics.h`
  - `src/DoctorDiagnostics.cpp`
  - `tools/cli_app.cpp`
  - `tests/test_doctor_diagnostics.cpp`
  - `CMakeLists.txt`
  - `README.md`
  - `training/README.md`
  - `WORKSTREAM_TRACKER.md`
  - `CHECKPOINT.md`
  - `CHANGELOG.md`
  - `SKILL_PLAYBOOK.md`
- Exact checks run:
  - Red: `cmake --build build --config Debug --target unit_tests` failed
    before implementation on missing `daisyhost/DoctorDiagnostics.h`.
  - Green helper/CTest: `ctest --test-dir build -C Debug --output-on-failure -R "DoctorDiagnostics|DaisyHostCliDoctor"`
    passed `9/9`.
  - Release build: normalized-env `cmake --build build --config Release --target DaisyHostCLI unit_tests -- /m:1`
    passed.
  - Direct CLI proof: normalized-env
    `build\Release\DaisyHostCLI.exe doctor --build-dir build --source-dir . --config Release --json`
    returned `ok: true` with no blockers and expected CTest registrations.
  - Release focused CTest:
    `ctest --test-dir build -C Release --output-on-failure -R "DaisyHostCliDoctor|DoctorDiagnostics"`
    passed `9/9`.
  - Final gate: `cmd /c build_host.cmd` passed with Release `ctest`
    `269/269`.
  - Next-WP recommender:
    `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md`
    recommended `TF12 - Verification / build hardening`; runner-up was
    `WS10 - External state / debug surface`; explicit waits remain `WS9`,
    `WS11`, `WS12`, `WS13`, `TF17`, and `TF18`.
- Notes:
  - Existing dirty work is present in `CMakeLists.txt`, Subharmoniq source and
    tests, `tests/run_smoke.py`, vault files, submodules, and this tracker;
    TF15 preserved unrelated changes.
  - A direct doctor run in the unsanitized shell can correctly fail with
    `duplicate-path-env`; the final proof used the same normalized environment
    approach as the build wrapper.

Previous implementation iteration:

- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: TF8/TF9 Subharmoniq Field performance-control upgrade
- Manager-readable result:
  - Done: Subharmoniq now opens on a sequencer-first Field surface, with four
    performance pages: `Seq/Rhy`, `VCO`, `VCF`, and `VCA/Mix`. Field K1-K8
    consume the active page binding, while A1-B8 remain globally stable for
    sequencer steps, rhythm routing, quantize, sequencer octave, play/stop, and
    reset.
  - Why it matters: the Field control model now matches the approved
    sequencer-centered Subharmonicon workflow instead of burying sequence and
    rhythm control behind the older generic app-page cycle.
  - What it unlocks: Daisy Field users can keep transport/rhythm/sequence
    actions under fixed A/B keys while SW1/SW2 page through sound-shaping
    controls. `Quantize` and `Seq Oct` are now real page-editable host
    parameters, not dead VCO-page labels.
  - Out of scope: no firmware, DAW/VST3, hardware-voltage, mixed-board routing,
    or Patch-board DSP behavior was changed or manually validated.
- Affected surfaces:
  - `include/daisyhost/DaisySubharmoniqCore.h`
  - `src/DaisySubharmoniqCore.cpp`
  - `src/apps/SubharmoniqCore.cpp`
  - `tests/test_subharmoniq_core.cpp`
  - `tests/run_smoke.py`
  - `PROJECT_TRACKER.md`
- Exact checks run:
  - Debug build:
    `cmake --build build --config Debug --target unit_tests`:
    passed.
  - Targeted Debug behavior tests:
    `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SubharmoniqCoreTest|RenderRuntimeTest|HostModulationTest)"`:
    passed, `75/75`.
  - Standalone Debug build:
    `cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    passed.
  - Planned standalone smoke before helper update:
    `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --timeout-seconds 60`:
    failed because `tests/run_smoke.py` did not yet accept `--board`.
  - Updated explicit Field standalone smoke:
    `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --timeout-seconds 60`:
    passed for `board=daisy_field`, `app=torus`.
  - Subharmoniq-specific Field standalone smoke:
    `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --app subharmoniq --timeout-seconds 60`:
    passed.
- LED-state assumptions:
  - A1-A4 and B1-B4 light the current sequencer step while playing.
  - A5-A8 rhythm target LEDs use `0.0 = off`, `0.5 = blink` for one-sequencer
    routing, and `1.0 = on` for both sequencers.
  - B5 Quantize uses off for `Off`, blink for equal temperament, and on for
    just intonation; B6 Seq Oct uses off/blink/on for 1/2/5 octaves; B7 is on
    while playing; B8 remains off except reset press state handled by the host
    pressed-key overlay.
- Notes:
  - The earlier cutoff/audio-off report is covered by the existing
    `HostModulationTest.FieldExternalCutoffTargetsKeepAudibleSafetyFloor` and
    `RenderRuntimeTest.SubharmoniqFieldCutoffSurfaceControlDoesNotMuteAudio`
    checks in the targeted green run. No new root-cause code change was needed
    in this iteration because that safety floor was already present in current
    processor/modulation code.
  - Manual visual/audio validation in the standalone UI and real Daisy Field
    hardware validation were not performed.

Previous implementation iteration:

- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: Manager-language planning and closeout persistence
- Manager-readable result:
  - Done: the "manager terms first" rule is now persisted across the local
    agent rules, portfolio tracker, project ledger, checkpoint, changelog,
    skill playbook, and README entrypoint.
  - Why it matters: future WP plans and handoffs should be understandable to a
    manager before they require file names, class names, or test names.
  - What it unlocks: each future plan can explain what needs implementation
    and why; each closeout can explain what was implemented, what changed for
    users/agents/CI, what remains, what was not claimed, and what evidence
    proves the status.
  - Next: use this structure for `TF15` and later WP planning/closeout; keep
    this pass docs-only and do not infer new runtime, hardware, DAW, firmware,
    or CLI behavior from it.
- Affected surfaces:
  - `AGENTS.md` durable local rule for planning, handoff, and completion
  - `README.md` workspace entrypoint guidance
  - `WORKSTREAM_TRACKER.md` portfolio-rule wording
  - `PROJECT_TRACKER.md` source-of-truth manager-language rule and ledger
  - `CHECKPOINT.md` durable memory note
  - `CHANGELOG.md` durable workflow-history note
  - `SKILL_PLAYBOOK.md` skill-evidence wording rule
- Exact checks run:
  - Docs consistency:
    `Select-String -Path AGENTS.md,README.md,WORKSTREAM_TRACKER.md,PROJECT_TRACKER.md,CHECKPOINT.md,CHANGELOG.md,SKILL_PLAYBOOK.md -Pattern "Manager-Language|manager-readable|manager terms|what needs to be implemented|what was implemented|why it matters|what it unlocks|out of scope|next safe starting point"`:
    passed and found the rule across the high-priority docs.
  - Whitespace check:
    `git diff --check -- AGENTS.md README.md WORKSTREAM_TRACKER.md PROJECT_TRACKER.md CHECKPOINT.md CHANGELOG.md SKILL_PLAYBOOK.md`:
    passed with LF/CRLF warnings only.
- Notes:
  - Docs-only process update; no runtime/build/DAW/VST3/USB MIDI/Field
    hardware/firmware validation was required or run.
  - Existing dirty work across code, docs, submodules, and untracked files was
    preserved.

Previous implementation iteration:

- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: TF14 CLI gate diagnostics
- Manager-readable result:
  - Done: `DaisyHostCLI gate --json` now wraps the existing
    `build_host.cmd` gate and emits structured configure/build/ctest phase
    status, fixed target names, CTest totals, conservative blocker
    classifications, and capped output tail.
  - Why it matters: agents and CI can now get a small machine-readable handoff
    for the full host gate instead of manually mining long MSBuild and CTest
    logs.
  - Next: adopt `gate --json` as the first full-gate evidence command; keep
    source/build readiness expansion in `TF15` and keep GUI, DAW, firmware,
    and generic shell control out of DaisyHostCLI.
  - Next-WP recommender result after closeout: `TF15 - Doctor source/build
    readiness expansion`, runner-up `TF12 - Verification / build hardening`,
    overlap risk low if existing doctor fields stay backward compatible;
    explicit waits `WS9`, `WS11`, `WS12`, `WS13`, `TF17`, and `TF18`.
- Affected surfaces:
  - `include/daisyhost/GateDiagnostics.h` pure diagnostics contract
  - `src/GateDiagnostics.cpp` phase, CTest, blocker, output-tail, and JSON
    payload helpers
  - `tools/cli_app.cpp` `gate` command and shared external-command capture
  - `tests/test_gate_diagnostics.cpp` parser/classifier coverage
  - `CMakeLists.txt` CLI helper/test wiring
  - tracker/checkpoint/changelog/readme/training/skill-playbook closeout docs
- Exact checks run:
  - Red build:
    `cmake --build build --config Debug --target unit_tests` failed before
    implementation on missing `daisyhost/GateDiagnostics.h`.
  - Green targeted build:
    normalized-env `cmake --build build --config Debug --target unit_tests`:
    passed.
  - Targeted diagnostics tests:
    `ctest --test-dir build -C Debug --output-on-failure -R "GateDiagnostics|CliPayloads"`:
    passed `17/17`.
  - Release CLI/unit build:
    normalized-env `cmake --build build --config Release --target DaisyHostCLI unit_tests`:
    passed.
  - Direct CLI gate proof:
    `build\Release\DaisyHostCLI.exe gate --source-dir . --build-dir build --config Release --json`:
    passed with JSON `ok: true`, phases `passed/passed/passed`, no blockers,
    and CTest `244/244`.
  - Diagnostic failure proof:
    `build\Release\DaisyHostCLI.exe gate --source-dir . --build-dir build --config Release --skip-configure --skip-tests --json`
    first exposed a `locked-artifact` blocker for the standalone executable
    before a later rerun passed.
  - Full wrapper gate:
    first `cmd /c build_host.cmd` failed with transient Release payload-copy
    `PermissionError` in the first three tests after build and most CTest work
    had already succeeded; the requested rerun passed with Release `ctest`
    `244/244`.
- Notes:
  - Existing dirty work across host/editor/docs/submodules and untracked helper
    files was preserved.
  - TF14 did not expand `doctor`, add GUI automation, control a live plugin,
    validate DAW/VST3 behavior, flash firmware, or add generic shell/git
    wrapping.

Previous implementation iteration:

- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: TF9 board-generic editor surface completion
- Manager-readable result:
  - Done: DaisyHost editor-facing board policy now lives in `BoardProfile` for
    the supported `daisy_patch` and `daisy_field` boards.
  - Why it matters: the editor still behaves the same for Patch and Field, but
    board-specific panel names, selected-node hints, keyboard hints, trace
    rules, indicator visibility, and extended-surface visibility are no longer
    scattered through Patch-shaped UI branches.
  - Next: `WS11` can build Hub/scenario workflows on top of this foundation
    once scenario scope is explicit; new boards, routing presets, graph
    editing, firmware, hardware validation, and DAW/VST3 validation remain out
    of TF9 scope.
- Affected surfaces:
  - `include/daisyhost/BoardProfile.h` editor-surface policy model
  - `src/BoardProfile.cpp` Patch and Field profile metadata
  - `src/juce/DaisyHostPluginEditor.h` / `.cpp` board-neutral helper names and
    profile-backed hints, trace mode, indicator visibility, and extended
    surface visibility
  - `tests/test_board_profile.cpp` red/green coverage for supported board
    editor-surface policy
  - tracker/checkpoint/changelog/skill-playbook closeout docs
- Exact checks run:
  - Red build: `cmake --build build --config Debug --target unit_tests`
    failed before implementation on missing `BoardProfile::editorSurface` and
    `BoardEditorTraceMode`, proving the new board-editor metadata contract was
    not yet implemented.
  - Green build: `cmake --build build --config Debug --target unit_tests`:
    passed.
  - Targeted board checks:
    `ctest --test-dir build -C Debug --output-on-failure -R "BoardProfile|BoardControlMapping|DaisyFieldRSPLayout"`:
    passed `41/41`.
  - Broader affected Debug checks:
    `ctest --test-dir build -C Debug --output-on-failure -R "BoardProfile|BoardControlMapping|DaisyFieldRSPLayout|HostSessionState|EffectiveHostStateSnapshot"`:
    passed `59/59`.
  - Debug standalone build: raw MSBuild hit the known duplicate `Path` / `PATH`
    issue; normalized-env rerun of
    `cmake --build build --config Debug --target DaisyHostPatch_Standalone`
    passed.
  - Standalone smoke:
    `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --timeout-seconds 60`:
    passed for Patch/Torus and Field/Torus.
  - CLI board smoke:
    `build\Release\DaisyHostCLI.exe describe-board daisy_patch --json` and
    `build\Release\DaisyHostCLI.exe describe-board daisy_field --json`: passed.
  - Full gate: first `cmd /c build_host.cmd` hit a locked Release VST3 artifact;
    a stale headless `DaisyHostCLI` process was stopped. The rerun built but
    saw transient Release payload-copy permission errors in two `PolyOscCore`
    tests; focused `PolyOscCoreTest` rerun passed `6/6`, full Release `ctest`
    rerun passed `243/243`, and final `cmd /c build_host.cmd` passed with
    Release `ctest` `243/243`.
- Notes:
  - Existing dirty work across host/editor/docs/submodules and untracked helper
    files was preserved.
  - TF9 did not add new boards, routing presets, graph editing, firmware
    behavior, mixed-board rack behavior, Field hardware validation, manual GUI
    screenshot review, or DAW/VST3 validation.

Previous implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: repo-wide next-WP recommendation tool and handoff rule
- Manager-readable result:
  - Done: DaisyHost now has a checked repo-local recommender for the next
    suitable WP after a work package closes.
  - Why it matters: future closeouts can produce the next recommended WP,
    runner-up, explicit waits, dependency reason, overlap risk, and first safe
    slice from tracker truth instead of ad hoc memory.
  - Current recommendation from the tool: `TF9 - Board-generic editor surface`;
    runner-up `TF14 - CLI gate diagnostics`; overlap risk is low when limited
    to board/editor metadata and existing Patch/Field behavior; explicit waits
    `WS9`, `WS11`, `WS12`, `WS13`, `TF17`, and `TF18`.
- Affected surfaces:
  - `tools/suggest_next_wp.py` tracker parser and recommendation CLI
  - `tests/test_next_wp_suggester.py` parser/recommendation tests
  - `CMakeLists.txt` CTest registration as `DaisyHostNextWpSuggester`
  - `AGENTS.md` and `README.md` WP closeout workflow instructions
  - `WORKSTREAM_TRACKER.md` recommender invocation in the portfolio decision
    section
  - `CHECKPOINT.md`, `CHANGELOG.md`, `PROJECT_TRACKER.md`,
    `SKILL_PLAYBOOK.md` truth-bearing closeout docs
- Exact checks run:
  - Red direct test: `py -3 tests\test_next_wp_suggester.py`: failed before
    implementation because `tools/suggest_next_wp.py` was missing
  - Green direct test: `py -3 tests\test_next_wp_suggester.py`: passed,
    `2 tests`
  - Tool proof: `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md`:
    recommended `TF9 - Board-generic editor surface`, runner-up
    `TF14 - CLI gate diagnostics`, overlap risk low for board/editor-only
    scope, explicit waits `WS9`, `WS11`, `WS12`, `WS13`, `TF17`, and `TF18`
  - CMake configure: `cmake -S . -B build`: passed
  - CTest registration:
    `ctest --test-dir build -C Debug --output-on-failure -R "DaisyHostNextWpSuggester"`:
    passed `1/1`
  - Full host gate: `cmd /c build_host.cmd`: passed with Release `ctest`
    `233/233`, including `DaisyHostNextWpSuggester`
- Notes:
  - Tooling/docs-only product impact; no DaisyHost runtime, route semantics,
    editor layout, firmware, Field hardware, mixed-board behavior, or DAW/VST3
    validation changed or was claimed.
  - Existing dirty work across host/editor/docs/submodules was preserved.

Previous implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: WP manager-readable explanation process rule
- Manager-readable result:
  - Done: future DaisyHost WP plans, workstream updates, and completion
    handoffs must explain the work in non-specialist manager language.
  - Why it matters: project tracking can now show what changed, why it matters,
    what remains, and what is outside scope without requiring readers to know
    the implementation internals.
  - Next: apply this format to every new WP plan and completion row.
- Affected surfaces:
  - `AGENTS.md` iteration and handoff rules
  - `PROJECT_TRACKER.md` mandatory iteration testing notice and ledger
  - `WORKSTREAM_TRACKER.md` portfolio rules
  - `CHANGELOG.md` durable workflow history
- Exact checks run:
  - Docs consistency:
    `rg -n "manager-readable explanation|manager-readable WP|what is being implemented|what was done|why it matters|explicitly out of scope|No WP plan" AGENTS.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHANGELOG.md`:
    passed and found the new rule in all intended docs
  - Next-WP recommender:
    `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md`:
    recommended `TF9 - Board-generic editor surface`, runner-up
    `TF14 - CLI gate diagnostics`, explicit waits `WS9`, `WS11`, `WS12`,
    `WS13`, `TF17`, and `TF18`
  - Whitespace check:
    `git diff --check -- AGENTS.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHANGELOG.md`:
    passed with LF/CRLF warnings only
- Notes:
  - Docs-only process update; no runtime, firmware, Field hardware, or
    DAW/VST3 validation was required or run.
  - `CHECKPOINT.md` and `SKILL_PLAYBOOK.md` were reviewed but not changed:
    the durable forward rule belongs in the agent/process and tracker docs,
    and no skill evaluation changed.

Previous implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: WS10 external debug surface first slice
- Manager-readable result:
  - Done: existing `DaisyHostCLI snapshot --json` and `render --json` payloads
    now include an additive `debugState` object for external QA/tooling.
  - Why it matters: agents and diagnostics can read board, selected node,
    entry/output roles, routes, selected-node target cues, and render timeline
    target counts without reverse-engineering several payload arrays.
  - Next: keep using existing CLI commands; add new commands only when a real
    automation gap is logged.
- Affected surfaces:
  - `src/CliPayloads.cpp` debug-state serialization helpers for snapshot and
    render-result JSON
  - `tests/test_cli_payloads.cpp` red/green coverage for snapshot and render
    `debugState`
  - tracker/checkpoint/changelog/readme docs
- Exact checks run:
  - Red build: `cmake --build build --config Debug --target unit_tests`:
    passed
  - Red targeted test:
    `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"`:
    failed `2/8` on missing `debugState` in the new snapshot/render tests
  - Green build: `cmake --build build --config Debug --target unit_tests`:
    passed after a helper-ordering build failure was corrected
  - Green targeted payload tests:
    `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"`:
    passed `8/8`
  - Raw Debug CLI build:
    `cmake --build build --config Debug --target DaisyHostCLI` failed on the
    known duplicate `Path` / `PATH` MSBuild environment issue
  - Normalized-env Debug CLI build:
    `$pathValue = $env:Path; if([string]::IsNullOrEmpty($pathValue)) { $pathValue = $env:PATH }; [Environment]::SetEnvironmentVariable('PATH', $null, 'Process'); [Environment]::SetEnvironmentVariable('Path', $pathValue, 'Process'); cmake --build build --config Debug --target DaisyHostCLI`:
    passed
  - Direct Debug CLI snapshot:
    `build\Debug\DaisyHostCLI.exe snapshot --app multidelay --board daisy_field --selected-node node0 --json`:
    passed and returned `debugState`
  - Direct Debug CLI render:
    `build\Debug\DaisyHostCLI.exe render training\examples\field_node_target_surface_smoke.json --output-dir build\cli_smoke\ws10_field_node_debug --json`:
    passed, returned `debugState`, and produced checksum
    `cd30ef6ba7b7acdb`
  - Broader Debug regression subset:
    `ctest --test-dir build -C Debug --output-on-failure -R "(CliPayloadsTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest|HostSessionStateTest|LiveRackTopologyTest)"`:
    passed `76/76`
  - Full Release gate: `cmd /c build_host.cmd` passed with Release `ctest`
    `232/232`
- Notes:
  - WS10 did not add new CLI commands, routing presets, route semantics,
    topology changes, mixed-board behavior, firmware changes, Field hardware
    claims, or DAW/VST3 validation claims.
  - Existing top-level JSON fields remain unchanged; `debugState` is additive.
  - The suite count increased from `230/230` to `232/232` because this pass
    added two CLI payload tests.

Previous implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: TF10 routing contract hardening
- Manager-readable result:
  - Done: DaisyHost now has one tested routing rulebook for today's two-node
    audio rack, and both render/live consumers continue to agree on that
    contract.
  - Why it matters: future routing work can start from a stable foundation
    instead of rediscovering which route shapes are legal in each subsystem.
  - Next: `WS9` may plan new user-facing routing presets or semantics, but
    TF10 intentionally did not add them.
- Affected surfaces:
  - `LiveRackTopology` durable two-node audio contract wording and public
    contract comment
  - `tests/test_live_rack_topology.cpp` hardening coverage for validation-only
    plan builds, accepted config copying, failed-plan preservation, and durable
    unsupported-shape errors
  - tracker/checkpoint/changelog/workstream docs
- Exact checks run:
  - Red build: `cmake --build build --config Debug --target unit_tests` passed
    compilation with the new tests
  - Red targeted test:
    `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"`:
    failed `1/17` because unsupported-shape errors still said `sprint`
  - Green build: `cmake --build build --config Debug --target unit_tests`:
    passed
  - Green targeted topology:
    `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"`:
    passed `17/17`
  - Green affected consumers:
    `ctest --test-dir build -C Debug --output-on-failure -R "RenderRuntime|HostSessionState|EffectiveHostStateSnapshot"`:
    passed `51/51`
  - Full Release gate: `cmd /c build_host.cmd` passed with Release `ctest`
    `230/230`
- Notes:
  - TF10 did not add routing presets, graph editing, more than two rack nodes,
    non-audio routing, scenario/session schema changes, DAW automation changes,
    firmware changes, hardware claims, or DAW/VST3 validation claims.
  - The suite count increased from `227/227` to `230/230` because this pass
    added three route-plan hardening tests.

Previous implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: TF8/TF9 Field UI follow-up
- Affected surfaces:
  - Field board profile key legend and external CV/Gate/I/O placement
  - Field RSP key-legend layout helper and Mod lane display text helper
  - Subharmoniq Field key LED state provider
  - host/render Field surface LED snapshots and Field external cutoff safety
  - JUCE Field editor A/B legend and Mod lane detail labels
- Exact checks run:
  - Red build: `cmake --build build --config Debug --target unit_tests`
    failed as expected on missing
    `BuildDaisyFieldKeyMappingLegendLayout`,
    `daisyhost/HostModulationUiText.h`, and
    `SubharmoniqCore::GetFieldKeyLedValues`
  - Green build: `cmake --build build --config Debug --target unit_tests`:
    passed
  - Intermediate focused rerun:
    `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|DaisyFieldRSPLayoutTest|BoardControlMappingTest|HostModulationTest|RenderRuntimeTest|Subharmoniq)"`:
    failed `2/89`; the failures identified the existing default Subharmoniq
    Rhythm 1 target as `Seq1`/blink and one-node render manifests using
    top-level `finalParameterValues`
  - Green targeted rerun:
    `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|DaisyFieldRSPLayoutTest|BoardControlMappingTest|HostModulationTest|RenderRuntimeTest|Subharmoniq)"`:
    passed, `89/89`
  - Raw standalone build:
    `cmake --build build --config Debug --target DaisyHostPatch_Standalone`
    failed before editor compilation on the known duplicate `Path` / `PATH`
    MSBuild environment issue
  - Normalized-env standalone build:
    `$pathValue = $env:Path; if([string]::IsNullOrEmpty($pathValue)) { $pathValue = $env:PATH }; [Environment]::SetEnvironmentVariable('PATH', $null, 'Process'); [Environment]::SetEnvironmentVariable('Path', $pathValue, 'Process'); cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    passed and built
    `build/DaisyHostPatch_artefacts/Debug/Standalone/DaisyHost Patch.exe`
  - Standalone smoke:
    `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --timeout-seconds 60`:
    passed for `board=daisy_patch, app=torus` and
    `board=daisy_field, app=torus`
  - Whitespace check: `git diff --check`: passed with existing LF/CRLF
    warnings
- Notes:
  - Field key LED values are now app-provided with the host/render snapshots
    taking the maximum of momentary pressed state and app state. Subharmoniq
    uses `0.0` off, `0.5` blink, and `1.0` on for rhythm targets,
    quantize/octave modes, play state, and active sequencer steps while
    playing.
  - The cutoff/audio-off root cause was Field external controls/modulation
    being able to drive cutoff-like destinations to normalized `0.0`, which
    collapses the audible filter path. The fix applies a Field external
    safety floor of `0.08` for cutoff-like target ids while leaving ordinary
    parameter/menu/automation ranges intact.
  - Manual visible GUI inspection, real audio listening, Field hardware LED
    validation, DAW/VST3 validation, and physical CV voltage validation were
    not performed in this iteration.

Previous partial implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: TF11 node-targeted event surface expansion
- Affected surfaces:
  - `RenderRuntime` executed-timeline readback for resolved node targets
  - `CliPayloads` render-result JSON readback for nodes, routes, and executed
    timeline events
  - targeted `RenderRuntimeTest` and `CliPayloadsTest` red coverage
- Exact checks run:
  - Red build: normalized-env `cmake --build build --config Debug --target unit_tests -- /m:1` passed compilation
  - Red targeted tests: `ctest --test-dir build -C Debug --output-on-failure -R "(RenderRuntimeTest.ExecutedTimelineReportsResolvedTargetNodes|CliPayloadsTest.RenderPayloadIncludesNodeDebugReadback)"` failed for the expected missing resolved `targetNodeId` values and missing CLI render `nodes` array
  - Production-library build: normalized-env `cmake --build build --config Debug --target daisyhost_cli daisyhost_render -- /m:1` passed
  - Render executable build: normalized-env `cmake --build build --config Debug --target DaisyHostRender -- /m:1` passed
  - Direct render proof: `build\Debug\DaisyHostRender.exe .tmp\tf11_node_readback_scenario.json --output-dir .tmp\tf11_node_readback_render` passed and wrote a manifest whose executed timeline reports `targetNodeId` values `node0`, `node1`, `node1`, and `node1`
  - Standalone build: normalized-env `cmake --build build --config Debug --target DaisyHostPatch_Standalone -- /m:1` passed
  - Green targeted TF11 tests: `ctest --test-dir build -C Debug --output-on-failure -R "(RenderRuntimeTest.ExecutedTimelineReportsResolvedTargetNodes|CliPayloadsTest.RenderPayloadIncludesNodeDebugReadback)"` passed `2/2`
  - Green broader Debug contract subset: `ctest --test-dir build -C Debug --output-on-failure -R "(RenderRuntimeTest|CliPayloadsTest|HostSessionStateTest|EffectiveHostStateSnapshotTest|LiveRackTopologyTest)"` passed `71/71`
  - Full Debug CTest: `ctest --test-dir build -C Debug --output-on-failure` passed `227/227`
  - Full Release gate: `cmd /c build_host.cmd` passed with Release `ctest` `227/227`
- Notes:
  - TF11 did not change `LiveRackTopology`, routing presets, route validation
    semantics, mixed-board behavior, DAW automation, firmware adapters, Field
    hardware claims, or DAW/VST3 validation claims.
  - The implemented slice is source-backed, targeted-test-backed, and
    full-gate-backed in this checkout.

Previous partial implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: TF10 routing contract generalization
- Affected surfaces:
  - `LiveRackTopology` shared route-plan contract and validation delegation
  - `RenderRuntime` multi-node route validation/processing order integration
  - `DaisyHostPluginProcessor` live-rack processing order and routed-audio
    channel integration
  - `tests/test_live_rack_topology.cpp` route-plan red/green coverage
- Exact checks run:
  - Red: `cmake --build build --config Debug --target unit_tests` failed on
    missing `daisyhost::LiveRackRoutePlan` and
    `daisyhost::TryBuildLiveRackRoutePlan`
  - Green targeted topology: `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"` passed `14/14`
  - Green targeted render chain checks: `ctest --test-dir build -C Debug --output-on-failure -R "RenderRuntimeTest\\.(RunsTwoNodeAudioChainScenario|RunsReverseTwoNodeAudioChainScenario|RejectsUnsupportedCrossNodeCvRoute|RejectsAmbiguousMultiNodeImpulseWithoutTargetNodeId|FieldBoardShellPreservesFrozenTwoNodeRackContract)"` passed `5/5`
  - Green full render-runtime subset: `ctest --test-dir build -C Debug --output-on-failure -R "RenderRuntime"` passed `31/31`
  - Green targeted session/snapshot subset: `ctest --test-dir build -C Debug --output-on-failure -R "HostSessionState|EffectiveHostStateSnapshot"` passed `18/18`
  - Debug wrapper build without tests: `cmd /c build_host.cmd -Configuration Debug -SkipTests` passed and built `unit_tests`, `DaisyHostCLI`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone`
  - Full Release gate attempt: `cmd /c build_host.cmd` failed during
    `unit_tests` compilation because unrelated dirty tests reference missing
    `daisyhost/HostModulationUiText.h`,
    `SubharmoniqCore::GetFieldKeyLedValues`, and Field/RSP layout symbols
- Notes:
  - TF10 did not add routing presets, graph editing, more than two rack nodes,
    non-audio routing, scenario schema changes, DAW automation changes, or
    firmware/hardware claims.
  - Raw direct MSBuild still hit the known duplicate `Path` / `PATH` issue;
    wrapper commands remain the source of usable build evidence.
  - A later TF11 closeout reran the wrapper gate green at `227/227`, so the
    current checkout is full-gate-backed again. `WS9` should still wait for an
    explicit product decision rather than expanding route semantics by default.

Latest fully gate-backed implementation iteration:

- Date: 2026-04-27
- Thread: Local Codex thread
- Slice: WS8 rack UX productionization
- Affected surfaces:
  - `LiveRackTopology` operator-facing topology/role display helpers
  - rack role labels, selected-node context, topology direction copy, and
    board-aware selected-node target hint in the JUCE editor
  - processor role-label handoff through the existing selected-node API
  - rack/Field/RSP verification evidence and tracker closeout docs
- Exact checks run:
  - Red: normalized-env `cmake --build build --config Debug --target unit_tests -- /m:1` failed on missing `GetLiveRackTopologyDisplayLabel` and `GetLiveRackNodeRoleDisplayLabel`
  - Green build: normalized-env `cmake --build build --config Debug --target unit_tests -- /m:1`: passed
  - Green targeted rack/session/render/board tests: `ctest --test-dir build -C Debug --output-on-failure -R "(LiveRackTopologyTest|HostSessionStateTest|EffectiveHostStateSnapshotTest|BoardProfileTest|BoardControlMappingTest|RenderRuntimeTest)"`: passed, `96/96`
  - Green standalone build: normalized-env `cmake --build build --config Debug --target DaisyHostPatch_Standalone -- /m:1`: passed
  - Focused Field/RSP status check: `ctest --test-dir build -C Debug --output-on-failure -R "(DaisyFieldRSPLayoutTest|BoardProfileTest|BoardControlMappingTest|RenderRuntimeTest)"`: passed, `69/69`
  - Release render smoke: `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for MultiDelay, Torus, CloudSeed, Braids, Harmoniqs, VA Synth, PolyOsc, Daisy Field shell, Daisy Field native controls, Daisy Field extended surface, Daisy Field selected-node surface, and Daisy Field PolyOsc surface scenarios
  - Full host gate: `cmd /c build_host.cmd`: passed with Release `ctest` `216/216`, including standalone, render, and CLI smoke tests
- Notes:
  - The first full-gate attempt in this WS8 closeout hit `LNK1104` on the
    Release `DaisyHost Hub.exe` because a Hub process was still running; that
    process was stopped before the final successful wrapper run.
  - One later wrapper retry was aborted before it returned usable output; it
    is not counted as evidence.
  - Debug standalone builds still report existing JUCE/editor `C4702`
    unreachable-code warnings.
  - Manual visible GUI inspection, Field hardware validation, generated-adapter
    flashing, real CV voltage measurement, Field DAW/VST3 validation,
    DaisyHostController USB MIDI validation, mixed-board racks, and arbitrary
    firmware import were not performed or claimed for WS8.

Current decision-useful status:

- the planned `WS1` through `WS7` milestone set is now complete enough to
  freeze in this checkout
- `WS8` rack UX productionization is source-backed and gate-backed in this
  checkout: the two-node rack keeps the existing four audio-only presets but
  now exposes clearer topology labels, selected-node role labels, per-node
  context, topology direction copy, and Patch/Field selected-node target hints.
- the hosted-app set now includes:
  - `multidelay`
  - `torus`
  - `cloudseed`
  - `braids`
  - `harmoniqs`
  - `vasynth`
  - `polyosc`
  - `subharmoniq`
- Daisy Field board-support shell, host-side Field native controls, and Field
  extended host surface support are implemented and refined in this checkout
  through the existing board factory seam. Hub `Play / Test` now uses the
  startup-request handoff, render smoke proves selected-node Field surface
  evidence, Field K5-K8 avoid duplicate K1-K4 parameter targets where the apps
  expose explicit knob metadata, and Field UI lookup is more board-profile
  driven. Sprint F3 also adds `field/MultiDelay` as the first Daisy Field
  firmware adapter, with build and ST-Link flash/verify evidence from
  2026-04-25. The SubharmoniqField pass adds a portable
  `DaisySubharmoniqCore`, first-class `subharmoniq` hosted app, and
  build/QAE/ST-Link flash-verified `field/SubharmoniqField` firmware adapter on
  2026-04-26.
  Follow-up no-audio fixes add the internal tempo clock needed for `B7` play
  to produce rhythm-triggered envelopes without external clock, tune the
  default envelope/output/filter path to a host-tested audible level, and make
  Field K1-K8 pickup-style at startup so physical knob positions do not mute
  the safe patch. A later DaisyHost regression pass proves the `subharmoniq`
  Field surface path directly: host `A7` applies the Rhythm 3 action, host
  `B7` toggles play, both sequencers advance, and an offline `daisy_field`
  render produces non-silent stereo output after the same key sequence. The
  DaisyHost Field surface now separates hosted-app
  navigation from right-side drawer navigation: SW1/X and SW2/C rotate the
  active app menu, while right-side program pages stay mouse/debug controlled.
  Page 1 exposes latched CV targets for knob-controlled parameters, and
  CloudSeed now has Space, Motion, Arp, and Advanced app pages with the
  Advanced Field page limited to EQ/seed parameters that do not disable or mute
  audio input/output. A follow-up CV-generator stability fix prevents a latched
  CV target from also feeding the same app CV input lane and hides/skips
  audio-critical CV targets such as mix, input mix, output, mute/bypass/enabled,
  level, and volume. The Field CV target menu now exposes normal Field K targets
  as `Kx.1` and non-overlapping alternative/public-parameter K targets as
  `Kx.2`. The later oversized Page 1 modulation-bay experiment was reverted:
  RSP Page 1 is back to the compact split layout with Keyboard MIDI, octave,
  visible keyboard, and MIDI tracker on the left, and compact CV generator
  controls, live CV bars, and Gate 1/2 on the right. The build/status helper
  text remains in the host drawer rather than being moved as part of the
  reverted experiment.
  The new `field/DaisyHostController` target is also build/QAE/ST-Link
  flash-verified as a controller-only USB MIDI firmware path for driving
  DaisyHost via existing MIDI input and CC learn, with K1-K8 as CC 20-27,
  CV1-CV4 as CC 28-31, A1-B8 as notes 60-75, and SW1/SW2 as momentary
  CC 80/81.
  Hardware flashing and manual audio/control/CV validation
  remain pending. The
  adapter pipeline v0 now generates a build/QAE-verified
  `field/MultiDelayGenerated` project from a checked-in JSON spec and provides
  a read-only firmware portability audit. Manual Field audio/control/CV
  hardware validation, generated-adapter flashing, deeper Field-specific app
  ergonomics, mixed-board rack behavior, and manual DAW/VST3 validation remain
  deferred. Field-specific follow-on work is tracked in
  `FIELD_PROJECT_TRACKER.md`.
- the next forward portfolio is tracked in `WORKSTREAM_TRACKER.md` and mirrored
  below in this file
- DaisyHost now has a host-side v1 modulation-lane architecture:
  - destinations are eligible continuous hosted-app parameters
  - each destination has up to four lane slots
  - sources are `CV 1-4` and `LFO 1-4`
  - evaluation is native-unit sum plus clamp over the stored base parameter
  - hosted apps expose transient effective-parameter override hooks so base
    values remain controlled by knobs/menu/automation/session
  - session format is `version 6` with legacy v1-v5 sessions loading with no
    modulation lanes
  - snapshots and CLI JSON include selected destination, lanes, live source
    value, native contribution, base/result native values, normalized result,
    and clamped status
  - the Field drawer labels are now `Play`, `Mod`, and `Rack`; `Mod` provides
    destination selection and compact lane rows
- TF12 verification/build hardening is implemented as a docs and adoption pass:
  the current build target list includes `DaisyHostCLI`, the documented
  agent/CI CLI sequence passed directly, and no new CLI commands were added.
- TF10 routing-contract foundation is complete: shared `LiveRackRoutePlan`
  construction is source-backed, targeted-test-backed, and full-gate-backed.
  Manager-readable result: DaisyHost now has one tested rulebook for today's
  two-node audio rack, while `WS9` remains the place for new routing presets or
  product semantics.
- TF11 node-targeted event/readback expansion has an initial full-gate-backed
  slice: render manifests now report resolved target-node identity for inferred
  node-scoped timeline events, and CLI render-result JSON exposes render
  `nodes`, `routes`, and `executedTimeline` debug readback.
- TF13 CLI activity logging and scope governance is implemented as a docs-only
  layer: future DaisyHostCLI commands should be promoted from logged agent/CI
  pain, not from speculative convenience.

Historical note: earlier DaisyHost docs drifted on host test counts and related
verification assertions. Those stale numeric claims are now removed or
relabeled until the runtime commands are rerun in a future iteration.

## Mandatory Iteration Testing Notice

Every implementation iteration must record:

- date
- thread or agent
- affected slice or files
- manager-readable explanation:
  - what is being implemented or what was done
  - why it matters
  - what it unlocks
  - what depends on it or blocks it
  - what remains
  - what is explicitly out of scope
  - what evidence proves the claim
  - what the next safe starting point is
- exact tests, builds, or checks run
- result
- blockers and handoff

No iteration is complete without test evidence.
No WP plan or closeout is complete without manager-readable explanation; the
technical notes must still remain precise enough for the next agent to continue.

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
| `FIELD_PROJECT_TRACKER.md` | Field-specific status, implementation sprint backlog, and separate DAW/VST3 validation todo. | Review before any `daisy_field` refinement, hardware, firmware, or Field validation work; update when Field status or sprint order changes. |
| `WORKSTREAM_TRACKER.md` | Forward-looking post-WS7 workstream portfolio and parallelization plan. | Review when the next workstream order, dependencies, or staffing-safe joins change; mirror materially relevant updates in `PROJECT_TRACKER.md`. |
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
- Workstream 6 is now landed: `multidelay` and `cloudseed` expose named
  MetaControllers through the mirrored menu/drawer path, and both `cloudseed`
  and `braids` are now treated as first-class supported hosted apps.
- The local checkout now also source-backs two more first-class hosted apps
  without processor/editor rewrites:
  - `harmoniqs` as an additive MIDI/gate instrument with `Spectrum` /
    `Envelope` page remapping
  - `vasynth` as a seven-voice MIDI-first subtractive synth with `Osc` /
    `Filter` / `Motion` canonical pages and checked-in render scenarios
- The local checkout now has a standard host-build wrapper:
  `build_host.cmd` -> `build_host.ps1`.
- The local checkout now has a native `DaisyHostCLI.exe` for agent/CI use:
  app/board/input discovery, app/board JSON descriptions, scenario validation,
  offline render, effective-state snapshot, render-smoke delegation, and
  build-artifact doctor checks.
- The local checkout now source-backs `subharmoniq` as a
  Subharmonicon-inspired portable core and hosted app with six oscillator
  sources, two four-step sequencers, four rhythm dividers, quantize modes, and
  a build/QAE/ST-Link flash-verified `field/SubharmoniqField` adapter. This is
  not fully hardware-validated yet; manual Field audio/control/CV/MIDI/OLED/LED
  validation remains pending.
- External DAW/VST3 load validation is intentionally deferred until a
  post-Workstream-7 manual pass.
- Earlier Workstream 7 groundwork remains landed in the host-side contracts:
  the later visible-rack sprint builds on the already-source-backed
  node/route-aware session/render surfaces from the previous 2026-04-23 pass.
- The operator-facing WS7 sprint is now source-backed, build-backed, and
  smoke-backed, and now also fully Release-gate-backed:
  - the live plugin now runs exactly two hosted nodes (`node0`, `node1`)
  - the editor exposes a visible two-node rack header, selected-node
    highlighting, per-node app selectors, and four audio-only topology presets
  - Patch controls, drawer/menu actions, CV/gate/test-input controls, keyboard
    MIDI, and the five-slot DAW automation bank now target the selected node
  - a board-id-based factory seam now creates board profiles instead of
    hardcoding Patch construction in the processor path
  - session/save state moved to `HostSessionState` v5 with rack globals
    (`boardId`, `selectedNodeId`, `entryNodeId`, `outputNodeId`)
  - render events now support `targetNodeId` for multi-node non-ID-scoped
    events and the render runtime now proves both forward and reverse serial
    two-node audio chains
- Daisy Field board-support shell, host-side Field native controls, and Field
  extended host surface support are now implemented:
  - `daisy_patch` remains the default board
  - `daisy_field` is accepted by Hub, session, standalone startup, render, and
    smoke paths
  - Field K1-K4 mirror the selected node's current Patch page bindings
  - Field K5-K8 map to the next four automatable selected-node parameters by
    `importanceRank`, with missing targets disabled
  - Field CV1-CV4 and Gate In reuse the existing host CV/gate paths
  - Field A1-B8 emit 16 chromatic MIDI notes from the selected node's current
    keyboard octave
  - Field CV OUT 1-2 are host-side derived monitor outputs for the K5/K6 mapped
    parameters, reported as normalized values and `0..5V` evidence
  - Field SW1/SW2 are host-side momentary utility triggers for the first two
    selected-app utility menu actions
  - Field key LEDs, switch LEDs, Gate In LED, and Gate Out LED are derived,
    non-persisted indicators carried through snapshots, render manifests, the
    panel UI, and smoke evidence
  - rack topology, hosted-app DSP behavior, and selected-node routing remain
    frozen
- First Field firmware adapter is now present under `field/MultiDelay`:
  - it uses `daisy::DaisyField` and the shared
    `DaisyHost/src/apps/MultiDelayCore.cpp`
  - firmware `make`, QAE validation, and ST-Link `make program` flash/verify
    passed on 2026-04-25
  - it is flash-verified, not yet fully manually hardware-validated; the
    remaining hands-on checklist is audio I/O, K1-K5, K6-K8 no-op behavior,
    CV1, CV OUT 1/2 voltage behavior, SW1/SW2, LEDs, OLED, and dropout
    observation
- Adapter pipeline v0 is now present:
  - `tools/generate_field_adapter.py` generates a Daisy Field firmware adapter
    from `tools/adapter_specs/field_multidelay.json`
  - `field/MultiDelayGenerated` builds and passes QAE validation
  - `tools/audit_firmware_portability.py` classifies firmware projects as
    `portable-core-ready`, `needs-core-extraction`, or `not-supported-by-v0`
  - v0 remains semi-automatic and shared-core based; it does not translate
    arbitrary libDaisy firmware into DaisyHost
- The next portfolio is now a true parallel split between product-value
  workstreams and technical-foundation workstreams; see
  `WORKSTREAM_TRACKER.md` and the mirrored section below.

## Workstream Tracker Mirror

This section mirrors `WORKSTREAM_TRACKER.md` so the per-iteration ledger and
the next portfolio remain readable in one file.

### Baseline

- `WS1` through `WS7` planned milestone scope is complete enough to freeze in
  this checkout.
- Latest full host gate:
  `cmd /c build_host.cmd` passed on 2026-04-28 and Release `ctest` passed
  `269/269`, including `DaisyHostCliDoctor`.
- `WS8` rack UX productionization is implemented against the frozen two-node
  rack baseline without changing the four existing audio-only topology
  presets.
- Daisy Field board-support shell, host-side Field native controls, and Field
  extended host surface support are now implemented:
  `daisy_field` flows through Hub, session, standalone startup, render, and
  smoke paths while the rack remains frozen, including host-side Field
  outputs/switches/LEDs, startup-request launch planning, and selected-node
  Field surface render evidence.
- The TF12 adoption slice is implemented as docs plus verification hardening:
  `DaisyHostCLI` is in the current build target list, CTest covers its core
  smoke commands, and the documented agent/CI smoke sequence passed directly.
- TF10 routing-contract foundation is complete: shared `LiveRackRoutePlan`
  construction is source-backed, targeted-test-backed, and covered by the
  latest `269/269` full host gate. Manager-readable result: DaisyHost now has
  one tested routing rulebook for today's two-node audio rack; richer routing
  presets still belong to explicit `WS9` product scope.
- WS10 now has a first external debug-surface slice: existing
  `DaisyHostCLI snapshot --json` and `render --json` outputs include additive
  `debugState` readback for board, selected node, entry/output node roles,
  routes, selected-node target cues, and render timeline target counts without
  adding new CLI commands or route semantics.
- TF13 now defines the DaisyHostCLI activity logger and scope governance layer:
  future CLI commands are tracked as evidence-backed automation needs before
  code-bearing work such as TF14/TF15 starts.
- TF14 CLI gate diagnostics is implemented: `DaisyHostCLI gate --json` wraps
  the existing `build_host.cmd` gate, reports configure/build/ctest phases,
  CTest totals, stable target names, capped output tail, and conservative
  known-blocker classifications without changing build semantics.
- TF15 doctor source/build readiness expansion is implemented by extending
  existing `doctor --json`, not by adding a new command. Manager-readable
  result: agents and CI can now distinguish source-root readiness, build-tree
  readiness, artifact readiness, CTest registration readiness, and duplicate
  `Path` / `PATH` environment hazards before deciding whether to run the gate.
  Doctor remains a preflight report only; it does not execute the gate, drive
  GUI/live/DAW workflows, flash firmware, or provide generic shell control.
- TF9 board-generic editor surface is complete: editor-facing board panel
  names, selected-node hint copy, keyboard hint copy, trace mode, indicator
  visibility, and extended-surface visibility now live in `BoardProfile` for
  `daisy_patch` and `daisy_field`. Manager-readable result: the editor still
  behaves the same for the supported boards, but future board-surface work can
  use profile metadata instead of reopening Patch-shaped UI branches. No new
  board, firmware, routing preset, graph editor, hardware, or DAW/VST3
  validation scope shipped in TF9.
- CLI usefulness tiers:
  `Essential` = needed to keep agent/CI verification reliable,
  `Useful` = repeated workflow improvement, `Nice-to-have` = defer until
  proven, `Rejected` = outside the thin offline CLI scope.

### Product Value Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Percent complete | Status |
|---|---|---|---|---|---|---|
| `WS8` | Rack UX productionization | Makes the visible 2-node rack feel shippable: clearer node context, stronger role labels, better selected-node feedback, and fewer operator mistakes. | **Done:** frozen `WS7` rack baseline was available and preserved. | `TF8`, `TF9`, `TF12`, `WS9` | `100%` | Implemented; automated gate green, manual visual/hardware/DAW validation not claimed |
| `WS9` | Richer live routing presets | Expands the rack past the current four audio-only presets without jumping to a freeform graph editor. | **Ready for explicit planning:** `TF10` is complete as the routing foundation, but route semantics remain owned by `WS9` scope.<br>`WS7` rack runtime; full-gate-backed `TF10` routing contract | `WS8`, `TF10`, `TF11`, `TF12` | `0%` | Planned; do not start without explicit routing-preset scope |
| `WS10` | External state / debug surface | Exposes the effective host state outside the processor for tooling, QA, diagnostics, and demos. | **First slice implemented:** additive CLI `debugState` now exists on `snapshot --json` and `render --json`.<br>Existing snapshot model; clearer node-targeted event rules from `TF11` | `TF11`, `TF12`, `WS8` | `25%` | First external debug-surface slice implemented; no new CLI commands |
| `WS11` | Hub + scenario workflow upgrade | Turns Hub into a launch surface for curated rack setups, saved scenarios, and repeatable operator flows. | **Blocked / not ready:** board-generic editor dependency is complete, but scenario inventory/readback and Hub workflow scope remain unimplemented.<br>Stable rack UX from `WS8`; board-aware editor foundation from `TF8` / `TF9`; future scenario work from `TF17` / `TF18` | `WS8`, `TF8`, `TF9`, `TF12` | `0%` | Planned after scenario workflow expectations settle |
| `WS12` | DAW-facing polish | Improves host-facing ergonomics and validates real VST3 behavior after the rack baseline is frozen. | **Blocked / not ready:** rack UX is implemented, but broader DAW-facing validation hardening remains partial.<br>Stable rack UX from `WS8`; verification hardening from `TF12` | `WS8`, `TF12` | `0%` | Planned after `TF12` / manual DAW validation scope settles |
| `WS13` | CLI-guided QA workflow adoption | Turns DaisyHostCLI into the routine agent/CI evidence entrypoint without making it a GUI, DAW, firmware, or generic shell controller. | **Partly ready:** `TF13` governance, `TF14` gate diagnostics, and `TF15` doctor readiness now exist; broader adoption should wait for routine workflow evidence.<br>`TF13`, implemented `TF14`, implemented `TF15`, future adoption evidence | `TF12`, `TF14`, `TF15` | `20%` | Useful; gate diagnostics and doctor readiness are available, blocked on repeated adoption evidence |

### Technical Foundation Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Percent complete | Status |
|---|---|---|---|---|---|---|
| `TF8` | Daisy Field board support | Adds `daisy_field` through the board factory seam plus host-side Field native controls, host-side Field outputs/switches/LEDs, and first shared-core-to-Field firmware adapter generation so Field work can ship without reopening Patch-only architecture. | **Good to go:** `WS7` freeze gate is green; remaining work is validation/follow-on scope, not a dependency blocker.<br>Green `WS7` freeze gate | `WS8`, `WS11`, `TF9`, `TF12` | `70%` | Extended host surface + adapter pipeline v0 implemented; manual Field validation remains |
| `TF9` | Board-generic editor surface | Removes remaining Patch-shaped assumptions from the editor and board rendering path. | **Done:** board-editor surface policy is now profile-backed for the supported Patch and Field boards.<br>Existing board seam; pairs naturally with `TF8` | `TF8`, `WS8`, `WS11` | `100%` | Complete; editor-facing board surface policy is profile-backed for Patch and Field |
| `TF10` | Routing contract generalization | Stabilizes route validation and internal graph rules so richer routing does not become a rewrite every sprint. | **Done:** the shared two-node audio routing rulebook is source-backed, targeted-test-backed, and covered by the latest full gate.<br>`WS7` rack/session/render baseline | `WS9`, `TF11`, `TF12` | `100%` | Complete foundation contract; `WS9` owns any new routing presets or product semantics |
| `TF11` | Node-targeted event surface expansion | Broadens the node-scoped event model for live/render/debug tooling beyond the current first-pass contract. | **In progress:** initial render/debug readback slice is source-backed, targeted-test-backed, and covered by the latest full gate.<br>`WS7` node-targeted runtime | `WS9`, `WS10`, `TF10` | `60%` | Initial node-target readback slice implemented |
| `TF12` | Verification / build hardening | Keeps `build_host.cmd`, smoke coverage, and checkout verification boring and repeatable. | **Good to go:** wrapper and smoke harness exist; remaining hardening is incremental.<br>Current wrapper and smoke harness | All workstreams | `40%` | Verification/adoption slice implemented; broader hardening remains |
| `TF13` | CLI activity logger and scope governance | Adds the lightweight evidence log and usefulness tiers that decide whether new DaisyHostCLI commands are essential, useful, nice-to-have, deferred, or rejected. | **Done:** tracker/docs evidence is sufficient; no runtime dependency.<br>Existing `PROJECT_TRACKER.md` ledger and DaisyHostCLI adoption sequence | `TF12`, `TF14`, `TF15`, docs-only Worker 3 slices | `100%` | Essential; implemented as docs-only governance |
| `TF14` | CLI gate diagnostics | Adds structured full-gate evidence and known-blocker classification so agents do not manually mine long MSBuild/CTest logs. | **Done:** implemented as a thin wrapper over `build_host.cmd`; source/build preflight readiness is now covered by implemented `TF15`.<br>`TF13`, `build_host.cmd`, current CTest smoke entries | `TF10`, `TF11`, `TF15` | `100%` | Essential; implemented with `gate --json` phase, CTest, target, blocker, and output-tail diagnostics |
| `TF15` | Doctor source/build readiness expansion | Extends existing `doctor --json` beyond artifact existence into environment, source/build readiness, CTest registration, and known Windows path hazards while preserving existing top-level fields. | **Done:** expanded as a preflight/readiness report, not a gate runner or new command.<br>`TF13`, existing `doctor`, CMake/CTest/smoke paths | `TF14`, `TF12` | `100%` | Essential / Useful; implemented with source/build/ctest/environment/blocker readiness and explicit out-of-scope boundaries |
| `TF16` | CLI render assertions | Lets CI fail directly on expected checksum, non-silence, route count, node ids, and executed timeline target-node readback. | **Ready but not urgent:** TF11 render-result JSON already exposes the evidence; add assertions only if repeated render-proof gaps appear.<br>`TF11` render debug payloads | `WS10`, `TF11`, `TF14` | `0%` | Useful; planned only after activity-log evidence |
| `TF17` | Scenario inventory and validation matrix | Lets agents discover checked-in scenarios and app/board/input validation status without manual repo searches. | **Deferred until workflow need is repeated:** scenario parser and examples exist, but Hub/scenario workflow may still change.<br>Training examples and scenario parser | `WS11`, `TF12`, `TF14` | `0%` | Useful; deferred until scenario workflow needs it |
| `TF18` | Scenario-backed snapshot/readback | Produces effective-state snapshots from actual scenario/rack setup without writing audio. | **Deferred:** render manifests already provide much of this evidence; no-audio scenario inspection must become a repeated need first.<br>`TF11`, future `WS10` external debug surface | `WS10`, `TF16` | `0%` | Nice-to-have; deferred |

### ASCII Parallelization View

```text
Time ------------------------------------------------------------------------>

Frozen baseline:
  [WS1-WS7 complete] ----> [Daisy Field extended host surface implemented]

Product track:
  [WS8 Rack UX implemented] -----------> [WS11 Hub + Scenario]
  [WS9 Routing Presets] ---------------> [WS12 DAW Polish]
  [WS10 External State / Debug]
  [WS13 CLI-guided QA Workflow] <------- [TF14/TF15 CLI diagnostics]

Technical track:
  [TF8 Daisy Field] -------------------> [TF9 Board-Generic Editor]
  [TF10 Routing Contract] -------------> [TF11 Node Events]
  [TF12 Verification Hardening] ------------------------------------------+
  [TF13 CLI Activity Logger] ----------> [TF14 Gate Diagnostics]
                                 \------> [TF15 Doctor Expansion]
                                                                         |
Cross-links:                                                             |
  TF10 -> WS9                                                            |
  TF11 -> WS10                                                           |
  TF8  -> WS11                                                           |
  TF14, TF15, TF16 -> WS13                                                |
  TF17 -> WS11                                                            |
  TF18 -> WS10                                                            |
  WS8  -> WS11, WS12                                                     |
  TF12 -> all parallel lanes --------------------------------------------+
```

## Historical Order Through WS7

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

### DaisyHostCLI Activity Log

Use this log when a DaisyHost iteration exposes a CLI-relevant signal. Add a
row only when a CLI command was used for evidence, a CLI command failed to
provide needed evidence, a manual fallback repeated a task the CLI should
eventually own, or a tempting CLI idea was rejected to prevent scope creep.

Pain types:
`missing-command`, `missing-payload`, `weak-doctor`, `slow-check`,
`log-parsing`, `ambiguous-evidence`, `rejected-scope`.

Frequency values: `first`, `repeated`, `recurring`.
Tier values: `essential`, `useful`, `nice-to-have`, `reject`.
Decision values: `log-only`, `plan`, `implement-next`, `implemented`,
`defer`, `reject`.

| Date / thread | Workstream | Intended CLI use | Manual fallback used | Pain type | Frequency | Evidence | Candidate CLI change | Tier | Decision |
|---|---|---|---|---|---|---|---|---|---|
| 2026-04-27 / Local Codex thread | `WS10` | Read selected-node rack/debug context directly from existing `snapshot --json` and `render --json` commands. | Manual correlation of top-level snapshot/render fields, node arrays, routes, and executed timeline. | `missing-payload` | `repeated` | TF11 made raw node/route/timeline data available, but external QA still needed a compact board/selected-node/role/target summary. | Additive `debugState` object on existing CLI JSON payloads, with no new command. | `useful` | `implemented` |
| 2026-04-27 / Local Codex thread | `TF10` / `TF11` | Summarize full host-gate and targeted verification failures as structured evidence. | Manual reading of `cmd /c build_host.cmd`, MSBuild, and CTest output in tracker rows. | `log-parsing` | `repeated` | TF10 full-gate attempt and TF11 closeout both required interpreting wrapper/CTest output, known dirty-test blockers, stale processes, and Path/PATH behavior. | `TF14` `gate --json` thin wrapper with conservative known-blocker classification. | `essential` | `implement-next` |
| 2026-04-28 / Local Codex thread | `TF14` | Use `DaisyHostCLI gate --json` as the structured full-gate evidence handoff. | Manual fallback still used once for direct `cmd /c build_host.cmd` parity and rerun proof. | `log-parsing` | `repeated` | `gate --json` returned `ok: true`, phases `passed/passed/passed`, no blockers, and CTest `244/244`; it also classified a transient standalone link lock as `locked-artifact`. | Keep `gate --json`; use activity-log evidence before adding further diagnostics. | `essential` | `implemented` |
| 2026-04-28 / Local Codex thread | `TF15` | Use `doctor --json` to preflight source/build/CTest/environment readiness before longer verification. | Previously manual checks for source-root files, build-tree setup, generated artifacts, expected CTest smoke registrations, and duplicate `Path` / `PATH` hazards. | `weak-doctor` | `repeated` | TF15 added source/build/ctest/environment/blocker readiness to existing `doctor --json`; direct normalized-env Release `doctor --json` returned `ok: true`, no blockers, and expected CTest registrations; final `cmd /c build_host.cmd` passed Release `ctest` `269/269`. | Keep expanded `doctor --json`; use recurring adoption evidence before adding further CLI surface. | `essential` | `implemented` |
| 2026-04-27 / Local Codex thread | `TF12` / `TF13` | Use `doctor --json` to decide whether checkout/build/source state is ready before longer verification. | Manual checks of artifact paths, CTest smoke entries, source/build paths, and known Windows environment hazards. | `weak-doctor` | `repeated` | Current `doctor` checks basic artifacts only; it does not explain duplicate `Path` / `PATH`, CTest registration, stale/missing configs, or source/build readiness. | `TF15` doctor source/build readiness expansion. | `essential` | `implement-next` |
| 2026-04-27 / Local Codex thread | `TF11` | Inspect render proof for node/route/timeline readback. | Direct render manifest and CLI render-result inspection. | `missing-payload` | `first` | TF11 added render-result JSON `nodes`, `routes`, and `executedTimeline`, reducing the immediate need for scenario-backed snapshots. | Keep `TF18` deferred unless no-audio scenario inspection becomes repeated. | `nice-to-have` | `defer` |
| 2026-04-27 / Local Codex thread | CLI scope governance | Consider GUI/live plugin/DAW/firmware control requests. | Existing manual/standalone/DAW/hardware validation policy remains outside DaisyHostCLI. | `rejected-scope` | `recurring` | Local docs keep DaisyHostCLI as a thin offline facade; manual DAW/VST3, live plugin control, firmware flashing, and GUI automation are environment-heavy or unsafe. | No DaisyHostCLI command; use separate manual or external harnesses if explicitly scoped. | `reject` | `reject` |

### Per-Iteration Ledger

| Date | Thread / agent | Workstream | Files / slice | Tests run | Docs reviewed | Blockers | Handoff |
|---|---|---|---|---|---|---|---|
| 2026-04-28 | Local Codex thread | TF15 doctor source/build readiness expansion | `include/daisyhost/DoctorDiagnostics.h`, `src/DoctorDiagnostics.cpp`, `tools/cli_app.cpp`, `tests/test_doctor_diagnostics.cpp`, `CMakeLists.txt`, `README.md`, `training/README.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Red: Debug `unit_tests` failed on missing `daisyhost/DoctorDiagnostics.h`; green: Debug `DoctorDiagnostics\|DaisyHostCliDoctor` CTest passed `9/9`; normalized-env Release `DaisyHostCLI unit_tests` built; normalized-env direct Release `doctor --json` returned `ok: true` with no blockers; Release `DaisyHostCliDoctor\|DoctorDiagnostics` CTest passed `9/9`; final `cmd /c build_host.cmd` passed Release `ctest` `269/269`; next-WP recommender selected `TF12`, runner-up `WS10`. | `AGENTS.md`, `README.md`, `training/README.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | Existing unrelated dirty Subharmoniq/vault/submodule/untracked work remains and was preserved. A direct unsanitized doctor run can correctly fail with `duplicate-path-env`; normalized-env proof passed. An accidental parallel Debug/Release build collided on an object lock, then Release reran cleanly. | Manager-readable result: TF15 is complete as a readiness/preflight expansion of existing `doctor --json`, not a new command or gate runner. It tells agents/CI whether the checkout/build tree is ready enough to verify; it does not claim GUI, live plugin, DAW/VST3, firmware flashing, hardware validation, routing changes, or generic shell control. Next safe starting point per recommender is a small `TF12` verification/build-hardening slice; runner-up is `WS10` only if a concrete external-debug consumer needs more. |
| 2026-04-28 | Local Codex thread | Manager-language planning and closeout persistence | `AGENTS.md`, `README.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Docs consistency `Select-String` over the touched high-priority docs found the manager-language rule terms; scoped `git diff --check -- AGENTS.md README.md WORKSTREAM_TRACKER.md PROJECT_TRACKER.md CHECKPOINT.md CHANGELOG.md SKILL_PLAYBOOK.md` passed with LF/CRLF warnings only. | `AGENTS.md`, `README.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | Docs-only process update; no runtime/build/DAW/VST3/USB MIDI/Field hardware/firmware validation was required or run. Existing dirty code/docs/submodule/untracked work was preserved. | Manager-readable result: DaisyHost plans must now say what needs implementation and why before technical detail; closeouts must say what was implemented, why it matters, what it unlocks, what remains, what was not claimed, evidence, and the next safe starting point. This process rule is now consumed by TF15 and later closeouts. |
| 2026-04-28 | Local Codex thread | TF14 CLI gate diagnostics | `include/daisyhost/GateDiagnostics.h`, `src/GateDiagnostics.cpp`, `tools/cli_app.cpp`, `tests/test_gate_diagnostics.cpp`, `CMakeLists.txt`, `README.md`, `training/README.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Red: Debug `unit_tests` failed on missing `daisyhost/GateDiagnostics.h`; green: normalized-env Debug `unit_tests` built; `ctest --test-dir build -C Debug --output-on-failure -R "GateDiagnostics\|CliPayloads"` passed `17/17`; normalized-env Release `DaisyHostCLI unit_tests` built; direct Release `gate --json` passed with phases passed/passed/passed and CTest `244/244`; final `cmd /c build_host.cmd` passed Release `ctest` `244/244`; scoped `git diff --check` passed with LF/CRLF warnings only; next-WP recommender selected `TF15`, runner-up `TF12`. | `AGENTS.md`, `README.md`, `training/README.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | A raw Debug CLI build initially hit the known duplicate `Path` / `PATH` MSBuild issue; normalized-env rerun passed. One diagnostic run classified a transient standalone `LNK1104` as `locked-artifact`; a standalone wrapper run later hit transient unit-payload `PermissionError`; final reruns passed. | Manager-readable result: TF14 is complete as a thin `gate --json` diagnostic wrapper over `build_host.cmd`. Next safe starting point is `TF15` doctor source/build readiness expansion; do not expand TF14 into GUI automation, live plugin control, DAW/VST3 validation, firmware flashing, or generic shell/git wrapping. |
| 2026-04-28 | Local Codex thread | TF9 board-generic editor surface completion closeout | `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `src/juce/DaisyHostPluginEditor.*`, `tests/test_board_profile.cpp`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Red: Debug `unit_tests` failed on missing `BoardProfile::editorSurface` / `BoardEditorTraceMode`; green: targeted Debug board checks passed `41/41`; broader affected Debug checks passed `59/59`; normalized-env Debug standalone build passed; Debug standalone smoke passed for Patch/Torus and Field/Torus; Release CLI `describe-board` checks passed for Patch and Field; after transient locked-artifact and payload-copy reruns, final `cmd /c build_host.cmd` passed Release `ctest` `243/243`. | `AGENTS.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | Existing dirty work spans editor/profile/docs/submodules and was preserved. No new board, routing preset, graph editor, firmware behavior, Field hardware claim, mixed-board rack behavior, manual GUI screenshot review, or DAW/VST3 validation is in scope. | Manager-readable result: TF9 is complete because board-facing editor policy now lives in `BoardProfile` for existing Patch and Field boards, while the editor consumes that policy without changing supported behavior. Next safe starting point is `TF15` doctor source/build readiness expansion or explicit `WS11` scenario workflow planning, not more TF9 implementation. |
| 2026-04-28 | Local Codex thread | TF9 board-generic editor surface completion claim | `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `src/juce/DaisyHostPluginEditor.*`, `tests/test_board_profile.cpp`, tracker/checkpoint/changelog docs after verification | Pre-implementation claim only; red/green evidence is superseded by the closeout row above. | `AGENTS.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, current dirty diff | Existing dirty work spans editor/profile/docs/submodules and must be preserved. No new board, routing preset, graph editor, firmware behavior, Field hardware claim, mixed-board rack behavior, or DAW/VST3 validation is in scope. | Active package is bounded to manager-readable TF9 completion: move remaining editor-facing board text/layout/rendering policy into board-profile metadata for existing `daisy_patch` and `daisy_field`, then prove Patch/Field behavior remains compatible through targeted tests and the full host gate. |
| 2026-04-27 | Local Codex thread | Repo-wide next-WP recommendation workflow | `tools/suggest_next_wp.py`, `tests/test_next_wp_suggester.py`, `CMakeLists.txt`, `AGENTS.md`, `README.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Red: `py -3 tests\test_next_wp_suggester.py` failed before implementation because the recommender module was missing; green: `py -3 tests\test_next_wp_suggester.py` passed `2 tests`; tool proof `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md` recommended `TF9`, runner-up `TF14`, low board/editor-only overlap risk, waits `WS9`, `WS11`, `WS12`, `WS13`, `TF17`, `TF18`; `ctest --test-dir build -C Debug --output-on-failure -R "DaisyHostNextWpSuggester"` passed `1/1`; `cmd /c build_host.cmd` passed Release `ctest` `233/233`. | `AGENTS.md`, `README.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | No code/runtime blocker. Existing dirty work remains across host/editor/docs/submodules and was preserved. No runtime, route semantics, editor layout, firmware, Field hardware, mixed-board behavior, or DAW/VST3 validation changed or was claimed. | WP closeouts now have a tested repo-local command for the next decision: `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md`. Current result: next WP `TF9 - Board-generic editor surface`; runner-up `TF14 - CLI gate diagnostics`; first safe slice is replacing the next Patch-shaped editor/layout assumption with board-profile metadata for existing `daisy_patch` and `daisy_field` behavior only. |
| 2026-04-27 | Local Codex thread | WP manager-readable explanation process rule | `AGENTS.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHANGELOG.md` | Docs consistency: `rg -n "manager-readable explanation\|manager-readable WP\|what is being implemented\|what was done\|why it matters\|explicitly out of scope\|No WP plan" AGENTS.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHANGELOG.md` passed; next-WP recommender `py -3 tools\suggest_next_wp.py --tracker WORKSTREAM_TRACKER.md` recommended `TF9` with runner-up `TF14`; scoped `git diff --check -- AGENTS.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHANGELOG.md` passed with LF/CRLF warnings only. | `AGENTS.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, current diff | Docs-only process update; no runtime, firmware, Field hardware, DAW/VST3, or manual validation was required or run. `CHECKPOINT.md` and `SKILL_PLAYBOOK.md` were intentionally left unchanged because current verification truth and skill UF evidence did not change. | Future WP plans and completion rows must include manager-language done/implementing, why-it-matters, remaining-work, and out-of-scope explanations. Recommender result: next WP is `TF9 - Board-generic editor surface`; first safe slice is replacing the next Patch-shaped editor/layout assumption with board-profile metadata for existing `daisy_patch` and `daisy_field` behavior only. |
| 2026-04-27 | Local Codex thread | WS10 external debug surface first slice | `src/CliPayloads.cpp`, `tests/test_cli_payloads.cpp`, tracker/checkpoint/changelog/readme docs | Red: `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"` failed `2/8` on missing `debugState`; green: `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"` passed `8/8`; direct Debug CLI `snapshot --json` and `render --json` checks passed and returned `debugState`; broader Debug subset passed `76/76`; `cmd /c build_host.cmd` passed Release `ctest` `232/232`. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `README.md`, `training/README.md`, current dirty diff | Raw Debug `DaisyHostCLI` build hit the known duplicate `Path` / `PATH` MSBuild issue; normalized-env rerun passed. Existing dirty work across host/editor/docs/submodules was preserved. No new CLI command, route semantics, topology preset, mixed-board behavior, firmware, Field hardware, or DAW/VST3 validation was added or claimed. | WS10 first slice is implemented as additive external debug readback on existing CLI JSON payloads. Next WS10 work should wait for a concrete external-debug consumer that needs more than `debugState`. |
| 2026-04-27 | Local Codex thread | TF10 routing contract hardening | `include/daisyhost/LiveRackTopology.h`, `src/LiveRackTopology.cpp`, `tests/test_live_rack_topology.cpp`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Red: `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"` failed `1/17` on sprint-era unsupported-shape wording. Green: `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"` passed `17/17`; `ctest --test-dir build -C Debug --output-on-failure -R "RenderRuntime\|HostSessionState\|EffectiveHostStateSnapshot"` passed `51/51`; `cmd /c build_host.cmd` passed Release `ctest` `230/230`. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | Existing dirty work remains across host/editor/docs/submodules and was preserved. No DAW/VST3, firmware, hardware, new route preset, graph editor, non-audio route, more-than-two-node route, or schema validation was performed or claimed. | TF10 is complete as a foundation contract. Manager-readable next step: use `WS9` for any new user-facing routing choices, because that is product design work rather than route-rule hardening. |
| 2026-04-27 | Local Codex thread | TF13 CLI activity logger and scope governance | `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md` | Docs consistency passed: `rg -n "TF13\|TF14\|TF15\|TF16\|TF17\|TF18\|WS13\|DaisyHostCLI Activity Log\|CLI verification" WORKSTREAM_TRACKER.md PROJECT_TRACKER.md`; `rg -n "classDef\|TF13\|TF14\|TF15\|WS13\|mermaid" WORKSTREAM_TRACKER.md PROJECT_TRACKER.md`; scoped `git diff --check -- WORKSTREAM_TRACKER.md PROJECT_TRACKER.md` passed with LF/CRLF warnings only. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, current diff | Docs-only pass; no runtime/build/DAW/VST3/USB MIDI/Field hardware/firmware validation required or run. Existing dirty worktree and unrelated submodule/untracked dirt must be preserved. | TF13 adds the CLI usefulness tiers, activity-log schema, seed evidence, WS/TF rows, and Mermaid/ASCII relationships. TF14/TF15 remain planned code-bearing follow-ups, not implemented diagnostics. |
| 2026-04-27 | Local Codex thread | TF10 routing contract generalization | `include/daisyhost/LiveRackTopology.h`, `src/LiveRackTopology.cpp`, `src/RenderRuntime.cpp`, `src/juce/DaisyHostPluginProcessor.cpp`, `tests/test_live_rack_topology.cpp`, tracker docs | Red: `cmake --build build --config Debug --target unit_tests` failed on missing `LiveRackRoutePlan` / `TryBuildLiveRackRoutePlan`. Green: `ctest --test-dir build -C Debug --output-on-failure -R "LiveRackTopology"` passed `14/14`; targeted two-node render chain CTest passed `5/5`; `ctest --test-dir build -C Debug --output-on-failure -R "RenderRuntime"` passed `31/31`; `ctest --test-dir build -C Debug --output-on-failure -R "HostSessionState\|EffectiveHostStateSnapshot"` passed `18/18`; `cmd /c build_host.cmd -Configuration Debug -SkipTests` passed. Later TF11 closeout reran the full wrapper green at `227/227`. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md`, current dirty diff | Existing dirty work is present across host/editor/docs and must be preserved. Raw direct MSBuild still needs the wrapper/sanitized environment. | TF10 route-plan contract is source-backed, targeted-test-backed, and now full-gate-backed; next safe step for routing is explicit `WS9` planning, not accidental route-semantics expansion. |
| 2026-04-27 | Local Codex thread | TF11 node-targeted event surface expansion | `src/RenderRuntime.cpp`, `src/CliPayloads.cpp`, `tests/test_render_runtime.cpp`, `tests/test_cli_payloads.cpp`, tracker docs | Red targeted tests failed for missing resolved `targetNodeId` and missing CLI render debug arrays; green targeted TF11 CTest passed `2/2`; broader Debug contract subset passed `71/71`; full Debug CTest passed `227/227`; full `cmd /c build_host.cmd` passed Release `ctest` `227/227`; direct render manifest proof reports resolved target nodes. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, current dirty diff in render/snapshot/CLI surfaces | Existing dirty work is present across host/editor/docs and must be preserved. Two earlier full-gate attempts failed only because stale standalone smoke processes were still running; those processes were stopped and the final wrapper rerun completed green. | Implemented slice is bounded to node-targeted non-routing event/readback/debug contracts for existing `node0` / `node1` racks, parallel-safe with TF10 routing-contract work. |
| 2026-04-27 | Local Codex thread | DaisyHost Field UI / RSP readability | `include/daisyhost/DaisyFieldRSPLayout.h`, `src/DaisyFieldRSPLayout.cpp`, `src/BoardProfile.cpp`, `src/juce/DaisyHostPluginEditor.*`, `src/juce/DaisyHostPluginProcessor.*`, `tests/test_board_profile.cpp`, `tests/test_daisy_field_rsp_layout.cpp`, `CMakeLists.txt`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Red: `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest\|DaisyFieldRSPLayoutTest)"` failed for missing key legend, missing two-row external control placement, and empty CV-card geometry. Green: `.\build_host.cmd -Configuration Debug -SkipTests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest\|BoardControlMappingTest\|DaisyFieldRSPLayoutTest)"` passed `38/38`; `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --timeout-seconds 60` passed for Patch/Torus and Field/Torus. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, Field editor/profile/mapping source | Raw direct MSBuild hit the known duplicate `Path` / `PATH` failure before tests compiled; wrapper build was used. Manual visible-GUI layout inspection was not run, so visual quality is build/test/smoke-backed only. Existing Debug `C4702` and DaisySP `C4244` warnings remain. | Field panel now has written A/B mapping text plus dynamic active-app key detail rows, hardware-style LED rendering with on/off and midpoint blink support, CV/Gate I/O grouped into two panel rows, and RSP Page 1 CV generators laid out as larger 2x2 cards with readable mode/waveform/target and slider text. |
| 2026-04-27 | Local Codex thread | Workstream Mermaid WS8+ diagram focus | `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md` ledger only | Docs consistency: `rg -n "WS1-WS7|base\\[|classDef done|Visual Status Diagrams" WORKSTREAM_TRACKER.md PROJECT_TRACKER.md`; final scoped `git diff --check -- WORKSTREAM_TRACKER.md` passed. | `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, current diff | Docs-only pass; no runtime/build/DAW/VST3/USB MIDI/hardware validation was run. The ASCII fallback and baseline prose still mention `WS1-WS7` by design. | Mermaid diagrams now focus on current/future `WS8+` and `TF8+` relationships, with the baseline node and unused green class removed. |
| 2026-04-27 | Local Codex thread | WS8 Rack UX productionization claim | `src/juce/DaisyHostPluginEditor.*`, possible rack-label helper/tests, tracker docs | Pre-implementation claim only; red/green evidence will be logged in a follow-up row before closeout. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `FIELD_PROJECT_TRACKER.md`, current dirty diff in rack/editor/processor files | Existing dirty work is present in editor/processor/RSP layout files and must be preserved. This WS8 pass must not add routing presets, mixed-board racks, DAW automation changes, firmware adapters, or hardware-validation claims. | Active package is now bounded to rack UX clarity: selected-node feedback, app identity, entry/output/inactive role copy, topology direction, and Patch/Field board-aware targeting cues. |
| 2026-04-27 | Local Codex thread | Workstream table progress/dependency readability | `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md` | Docs consistency: searched for current workstream table headers, TF12 wording, and Mermaid diagram blocks; final scoped `git diff --check -- WORKSTREAM_TRACKER.md PROJECT_TRACKER.md` passed. | `AGENTS.md`, `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md` current gate/status text, current diff | Docs-only pass; no runtime, DAW/VST3, USB MIDI, or hardware validation was run. Percent values are conservative manager-readable estimates from verified tracker evidence, not completion gates. Mermaid rendering was not browser-validated. | Workstream tables now include `Percent complete` and dependency readiness summaries, and `WORKSTREAM_TRACKER.md` has color-coded status/dependency diagrams. `TF12` is explicitly partial: adoption slice implemented, broader hardening remains. |
| 2026-04-26 | Local Codex thread | DaisyHost Subharmoniq Field A/B key UI enablement fix | `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp`, `src/juce/DaisyHostPluginProcessor.h`, `src/juce/DaisyHostPluginProcessor.cpp`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_board_control_mapping.cpp`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests -- /m:1` failed because `daisyhost::IsDaisyFieldKeyInteractive` did not exist. Green: `cmake --build build --config Debug --target unit_tests -- /m:1` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest\|SubharmoniqCoreTest\|RenderRuntimeTest.SubharmoniqFieldB7StartsAudioAfterA7RhythmEdit)"` passed `34/34`; `cmake --build build --config Debug --target DaisyHostPatch_Standalone -- /m:1` passed with existing editor C4702 warnings. | `DaisyExamples/AGENTS.md`, `DaisyExamples/LATEST_PROJECTS.md`, `START_FIRMWARE_SESSION.md`, `DaisyHost/AGENTS.md`, `README.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, Field editor/processor/mapping source | No code blocker. Visual standalone click-through was not manually inspected in a visible GUI session; the standalone target compiles the changed editor path. | Root cause: Field key buttons were enabled only when `GetFieldKeyMidiNote(i) >= 0`, so `subharmoniq` menu-action keys with `midiNote == -1` were drawn disabled. The editor now enables keys via the binding interactivity contract, preserving generic MIDI-note keys and enabling `subharmoniq` A/B menu-action keys. |
| 2026-04-26 | Local Codex thread | TF12 verification/build hardening adoption slice | `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `WORKSTREAM_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `README.md`, `CHANGELOG.md`, `training/README.md`, `SKILL_PLAYBOOK.md` | Green: `git status --short` and `git diff --name-status --` captured the broad dirty checkout; stale-count search reviewed `159/159`, `168/168`, `175/175`, `196/196`, and `197/197`; `cmd /c build_host.cmd` passed with Release `ctest` `202/202`; direct Release CLI checks passed for `doctor`, `list-apps`, `describe-app cloudseed`, `describe-board daisy_field`, `validate-scenario training\examples\multidelay_smoke.json`, `render training\examples\multidelay_smoke.json --output-dir build\cli_smoke\tf12_multidelay` with checksum `c9c3f665e6a0dd2b`, and `smoke --mode render`; final scoped `git diff --check` passed. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `CHANGELOG.md`, `training/README.md`, `SKILL_PLAYBOOK.md`, current dirty-tree and stale-count searches | Shared checkout remains broadly dirty across host, docs, firmware, submodules, build outputs, and untracked files; TF12 did not revert, stage, delete, or normalize unrelated work. Manual DAW/VST3, computer-side USB MIDI, and hands-on Field hardware validation were not run. | At that point the verification truth moved to `202/202`; DaisyHostCLI agent/CI usage is documented as adoption of existing commands, not a new command-expansion pass. Next safe step is to use the documented CLI sequence from CI/Codex workflows and add commands only after a real automation gap appears. |
| 2026-04-26 | Local Codex thread | DaisyHost Subharmoniq Field key/audio regression coverage | `tests/test_subharmoniq_core.cpp`, `tests/test_render_runtime.cpp`, tracking docs | Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(SubharmoniqCoreTest\|RenderRuntimeTest.SubharmoniqFieldB7StartsAudioAfterA7RhythmEdit)"` passed `11/11`, including direct core proof that `A7` then `B7` advances both sequencers and produces finite audio, plus render-runtime proof that `surface_control_set` for `field_key_a_7` then `field_key_b_7` produces non-silent stereo output through the DaisyHost `daisy_field` surface path. | `DaisyExamples/AGENTS.md`, `DaisyExamples/LATEST_PROJECTS.md`, `START_FIRMWARE_SESSION.md`, `field/SubharmoniqField/CONTROLS.md`, `DaisyHost/AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, render/core tests and runtime source | No code blocker in the automated host path. This did not include physical button presses or analog audio capture from hardware. | Host-side behavior is now covered: `A7` is a rhythm-assignment key, not a transport key; `B7` starts/stops play and produces audio in both direct app-core and DaisyHost Field surface render paths. Manual hardware confirmation still requires pressing physical `B7` and listening/measuring audio output. |
| 2026-04-26 | Local Codex thread | SubharmoniqField A/B row correction and DaisyHost Subharmoniq Field-key actions | `include/daisyhost/HostedAppCore.h`, `include/daisyhost/apps/SubharmoniqCore.h`, `src/apps/SubharmoniqCore.cpp`, `src/BoardControlMapping.cpp`, `src/CliPayloads.cpp`, `src/juce/DaisyHostPluginProcessor.cpp`, `src/RenderRuntime.cpp`, `tests/test_board_control_mapping.cpp`, `tests/test_subharmoniq_core.cpp`, `../field/SubharmoniqField/SubharmoniqField.cpp`, `../field/SubharmoniqField/CONTROLS.md`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed because `HostedAppPatchBindings` had no `fieldKeyMenuItemIds` / `fieldKeyDetailLabels` contract. Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(SubharmoniqCoreTest\|BoardControlMappingTest\\.SubharmoniqFieldKeysMapToDedicatedPerformanceActions\|BoardControlMappingTest\\.FieldKeysMapChromaticallyFromKeyboardOctave)"` passed `11/11`; `make` from `field/SubharmoniqField` passed with FLASH `126072 B` / `96.19%`; `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`; `make program` passed through OpenOCD with STLINK `V3J7M2`, target voltage `3.274751`, `** Verified OK **`, and target reset; a 2s OpenOCD check halted in Thread mode, not HardFault, then reset/reran the target. | `DaisyExamples/AGENTS.md`, `DaisyExamples/LATEST_PROJECTS.md`, `START_FIRMWARE_SESSION.md`, `field/SubharmoniqField/CONTROLS.md`, `DaisyHost/AGENTS.md`, `README.md`, `CHECKPOINT.md`, `CHANGELOG.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | No code blocker. Full wrapper host gate and hands-on button/audio validation were not run in this pass. | Physical Field `B7`/`B8` should now toggle play/reset; physical `A7`/`A8` should be rhythm keys again. DaisyHost `subharmoniq` on `daisy_field` now exposes A/B keys as app actions while other hosted apps keep chromatic MIDI key behavior. |
| 2026-04-26 | Local Codex thread | SubharmoniqField ST-Link flash verification | `../field/SubharmoniqField/README.md`, `../field/SubharmoniqField/CONTROLS.md`, `CHECKPOINT.md`, `FIELD_PROJECT_TRACKER.md`, `PROJECT_TRACKER.md`, `README.md`, `CHANGELOG.md` | Green: `make` from `field/SubharmoniqField` passed as up to date; `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`; `make program` passed through OpenOCD with STLINK `V3J7M2`, target voltage `3.263618`, `** Verified OK **`, and target reset. | `DaisyExamples/AGENTS.md`, `field/SubharmoniqField/README.md`, `field/SubharmoniqField/CONTROLS.md`, `DaisyHost/CHECKPOINT.md`, `DaisyHost/FIELD_PROJECT_TRACKER.md`, `DaisyHost/PROJECT_TRACKER.md`, `DaisyHost/README.md`, `DaisyHost/CHANGELOG.md` | The Field firmware is now build/QAE/ST-Link flash-verified, but manual audio/control/CV/MIDI/OLED/LED validation was not performed by Codex. | Complete the `field/SubharmoniqField/CONTROLS.md` hands-on checklist: B7 start, MIDI note, Gate In clock, K1-K8 pickup/pages, SW1/SW2, A/B keys, CV outs, Gate Out, OLED, LEDs, and audible output level. |
| 2026-04-26 | Local Codex thread | DaisyHost Field Page 1 rollback after oversized modulation-bay redesign | `include/daisyhost/ComputerKeyboardMidi.h`, `src/ComputerKeyboardMidi.cpp`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_computer_keyboard_midi.cpp`, tracking docs | Green: `rg -n "VisibleKeyboardStartForOctave|kKeyboardMinMidiNote|kKeyboardMaxMidiNote|BaseMidiNoteForOctave" include src tests` returned no matches; `git diff --check -- src\juce\DaisyHostPluginEditor.cpp include\daisyhost\ComputerKeyboardMidi.h src\ComputerKeyboardMidi.cpp tests\test_computer_keyboard_midi.cpp` passed with line-ending warnings only; `cmake --build build --config Debug --target unit_tests DaisyHostPatch_Standalone` passed with existing editor C4702 warnings; `ctest --test-dir build -C Debug --output-on-failure -R "(ComputerKeyboardMidiTest|BoardControlMappingTest|SignalGeneratorTest)"` passed `27/27`; full `cmd /c build_host.cmd` passed, including Release build of `unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` and Release `ctest` `196/196`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, Field drawer/editor source | No code blocker. This pass stayed host-side; no manual standalone screenshot/audio QA or firmware/hardware validation was run. | Reverted only the last Page 1 enlargement/keyboard-helper experiment. RSP Page 1 is back to the compact split layout; earlier stable work remains, including three RSP pages, SW1/X and SW2/C hosted-app navigation, `.1`/`.2` CV target labeling, non-overlapping default/alternative K targets, unsafe target filtering, and CV generator stability. |
| 2026-04-26 | Local Codex thread | DaisyHost Field Page 1 modulation-bay redesign and keyboard viewport | `include/daisyhost/ComputerKeyboardMidi.h`, `src/ComputerKeyboardMidi.cpp`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_computer_keyboard_midi.cpp`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed because `VisibleKeyboardStartForOctave` did not exist. Green: `cmake --build build --config Debug --target unit_tests DaisyHostPatch_Standalone` passed with existing editor C4702 warnings; `ctest --test-dir build -C Debug --output-on-failure -R "(ComputerKeyboardMidiTest|BoardControlMappingTest|SignalGeneratorTest)"` passed `28/28`; first full `cmd /c build_host.cmd` failed at link with `LNK1104` because live `DaisyHost Hub` / `DaisyHost Patch` processes locked Release artifacts; after closing those processes, full `cmd /c build_host.cmd` passed with Release `ctest` `192/192`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, Field drawer/editor source | Superseded by the rollback row above. This pass stayed host-side; no manual standalone screenshot/audio QA or firmware/hardware validation was run. | Historical note: this visual redesign made the host/RSP feel too large and heavy and was reverted. Do not treat the expanded modulation bay, Page 3 status move, or `VisibleKeyboardStartForOctave` helper as current behavior. |
| 2026-04-26 | Local Codex thread | DaisyHostController Field USB MIDI controller firmware | `../field/DaisyHostController/*`, `tests/test_daisyhost_controller_firmware.py`, tracking docs | Red: `py -3 -m pytest -q tests\test_daisyhost_controller_firmware.py` failed because `field/DaisyHostController/Makefile` did not exist. Green: the same pytest passed `1/1`; `make` from `field/DaisyHostController` passed with FLASH `98252 B` / `74.96%`; first QAE run hit Windows cp1252 Unicode output encoding before findings, then `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`. Flash follow-up: `make` exited 0 as up to date; `make program` passed through OpenOCD with STLINK V3, target voltage `3.238171`, `** Verified OK **`, and target reset. Manual follow-up: user reported all knobs, buttons, and keys displayed correctly on the Field screen. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `field/MultiDelay`, `field/SubharmoniqField`, `libDaisy` MIDI/USB MIDI headers | No code blocker. USB MIDI enumeration, DaisyHost standalone MIDI learn, DAW/VST3 routing, and computer-side MIDI validation were not run. | `field/DaisyHostController` is now ST-Link flash-verified with local Field control/OLED feedback checked. Next safe step: enable the Daisy USB MIDI input in DaisyHost standalone, verify tracker events, and bind at least one K knob through MIDI learn. |
| 2026-04-26 | Local Codex thread | DaisyHostCLI agent/CI facade | `include/daisyhost/CliPayloads.h`, `src/CliPayloads.cpp`, `tools/cli_app.cpp`, `CMakeLists.txt`, `build_host.ps1`, `tests/test_cli_payloads.cpp`, `README.md`, `training/README.md`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed on missing `daisyhost/CliPayloads.h`. Green: `.\build_host.cmd -Configuration Debug -SkipTests` built `unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`; `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"` passed `4/4`; direct Debug CLI checks for `list-apps`, `describe-app cloudseed`, `describe-board daisy_field`, `validate-scenario`, `snapshot`, `doctor`, `render`, and `smoke --mode render` passed; `ctest --test-dir build -C Debug --output-on-failure -R "DaisyHostCli"` passed `5/5`; full `cmd /c build_host.cmd` passed with Release build of `unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` and Release `ctest` `197/197`; Release `DaisyHostCLI.exe smoke --mode render --build-dir build --source-dir . --config Release --json` and `doctor --build-dir build --source-dir . --config Release --json` passed. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `training/README.md`, CLI/render/app/board/snapshot source | No code blocker. Raw MSBuild still hit the known duplicate `Path` / `PATH` issue before wrapper normalization. No firmware path changed. | Native DaisyHostCLI is build- and gate-verified for agent/CI discovery, app/board descriptions, scenario validation, offline render, effective-state snapshots, smoke delegation, and doctor checks. Next safe step is to use it from Codex/CI scenario-generation flows and add commands only when a real automation gap appears. |
| 2026-04-26 | Local Codex thread | DaisyHost Field CV target menu and generator visibility refinement | `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp`, `src/juce/DaisyHostPluginProcessor.cpp`, `src/juce/DaisyHostPluginEditor.*`, `tests/test_board_control_mapping.cpp`, tracking docs | Green: `cmake --build build --config Debug --target unit_tests DaisyHostPatch_Standalone` passed with existing editor C4702 warnings; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest|SignalGeneratorTest)"` passed `66/66`; full `cmd /c build_host.cmd` passed, including Release build of `unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` and Release `ctest` `187/187`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, relevant Field drawer/mapping/processor/editor source | No code blocker. This pass stayed host-side; no firmware or manual standalone visual/audio QA was run. | Field alternative K layout now excludes every default Field K target. CV target dropdowns show default safe targets as `Kx.1` and alternative safe targets as `Kx.2`; Page 1 lays out all four CV generators with compact mode/waveform/target controls plus Offset, Amp Vp, and Freq sliders while preserving the live CV bars. |
| 2026-04-26 | Local Codex thread | DaisyHost Field CV generator stability | `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp`, `src/juce/DaisyHostPluginProcessor.cpp`, `tests/test_board_control_mapping.cpp`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed because `ShouldForwardDaisyFieldCvInput` did not exist. Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SignalGeneratorTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest)"` passed `64/64`; full `cmd /c build_host.cmd` passed, including Release build of `unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` and Release `ctest` `185/185`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, relevant Field drawer/mapping/processor source | No code blocker. This pass stayed host-side; no firmware or hardware/manual audio validation was run. | CV generators now avoid the observed disappearing-output path by keeping latched knob/parameter CV lanes separate from app CV-input lanes and excluding unsafe audio-critical targets from default target choices/application. Next safe step is manual standalone audio QA with CloudSeed and MultiDelay CV generators. |
| 2026-04-26 | Local Codex thread | DaisyHost Field UI/control separation and CloudSeed safe advanced page | `include/daisyhost/HostedAppCore.h`, `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp`, `include/daisyhost/DaisyCloudSeedCore.h`, `src/DaisyCloudSeedCore.cpp`, `src/apps/CloudSeedCore.cpp`, `src/juce/DaisyHostPluginProcessor.*`, `src/juce/DaisyHostPluginEditor.*`, `tests/test_board_control_mapping.cpp`, `tests/test_daisy_cloudseed_core.cpp`, `tests/test_cloudseed_core.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed for missing `BuildDaisyFieldCvTargetOptions`, missing CloudSeed page fields, and missing Field knob override bindings. Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest)"` passed `59/59`; full `cmd /c build_host.cmd` passed, including Release build of `unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` and Release `ctest` `183/183`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md` | MP4 inspection was unavailable because `ffmpeg`, `cv2`, `imageio`, and `moviepy` were not installed. No firmware or hardware paths were touched. | SW1/X and SW2/C now drive hosted-app menu navigation only; RSP Page 1 has latched CV target dropdowns; CloudSeed exposes Space/Motion/Arp/Advanced pages. Next safe step is manual standalone visual QA of the new Page 1 CV target dropdowns. |
| 2026-04-26 | Local Codex thread | CloudSeed arpeggiator parameter-stepper | `src/DaisyCloudSeedCore.cpp`, `src/apps/CloudSeedCore.cpp`, `tests/test_daisy_cloudseed_core.cpp`, `tests/test_cloudseed_core.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/cloudseed_smoke.json`, tracking docs | Red: direct Debug payload for `DaisyCloudSeedCoreTest.*:CloudSeedCoreTest.*:RenderRuntimeTest.ProducesDeterministicCloudSeedRenderWithMenuActions` failed for missing `arp_*` canonical parameters, missing `node0/menu/arp/*` menu ids, and render rejecting `node0/menu/arp/enabled`. Green: `cmake --build build --config Debug --target unit_tests` passed; direct Debug payload for `DaisyCloudSeedCoreTest.*:CloudSeedCoreTest.*:RenderRuntimeTest.ProducesDeterministicCloudSeedRenderWithMenuActions:RenderRuntimeTest.PolyOscFieldSurfaceMapsK5ToWaveformOnly:RenderRuntimeTest.FieldExtendedSurfaceStateMirrorsOutputsSwitchesAndLeds` passed `13/13`; sanitized `cmake --build build --config Release --target unit_tests` passed; direct Release payload for the same filter passed `13/13`; sanitized `cmake --build build --config Release --target DaisyHostRender` passed; `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120` passed. Full gate attempt: `cmd /c build_host.cmd` failed before `ctest` because `DaisyHost Hub.exe` was locked at link with `LNK1104`; `Get-Process` showed running `DaisyHost Hub` PID `78928`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `WORKSTREAM_TRACKER.md`, CloudSeed core/app/test/render source | Full Release `ctest` was not reached because the Release Hub executable was held open by a running process. Raw Release `DaisyHostRender` first hit the known duplicate `Path` / `PATH` MSBuild issue and passed after the documented one-Path sanitization. | CloudSeed now has a deterministic parameter-based arpeggiator, not a MIDI-note arpeggiator. Next safe step is to close the running `DaisyHost Hub.exe` process and rerun `cmd /c build_host.cmd` for a full aggregate gate. |
| 2026-04-26 | Local Codex thread | SubharmoniqField audible-default no-audio follow-up | `src/DaisySubharmoniqCore.cpp`, `tests/test_subharmoniq_core.cpp`, `../field/SubharmoniqField/SubharmoniqField.cpp`, `../field/SubharmoniqField/README.md`, `../field/SubharmoniqField/CONTROLS.md`, tracking docs | Red: tightened `SubharmoniqCoreTest.PlayToggleRunsInternalClockAndProducesAudio` failed after the earlier internal-clock fix because peak was only `0.00323891826` vs required `0.02`, and one-second energy was `0.383889765` vs required `1.0`. Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R SubharmoniqCoreTest` passed `6/6`; `make` from `field/SubharmoniqField` passed with FLASH `124520 B` / `95.00%`; `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`; `cmd /c build_host.cmd` passed with Release `ctest` `175/175`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `field/SubharmoniqField/README.md`, `field/SubharmoniqField/CONTROLS.md` | Hardware flashing/manual audio checks were not run in this pass. The provided MP4 could not be inspected with `ffprobe` because the tool is not installed, but local source/test evidence identified a host-reproducible weak-audio path and a firmware startup knob-muting path. | Next safe step is `make program` plus the `field/SubharmoniqField/CONTROLS.md` manual hardware checklist: B7 start, MIDI note, Gate In clock, K1-K8 pickup/pages, SW1/SW2, A/B keys, CV outs, OLED, LEDs, and audible output level. |
| 2026-04-26 | Local Codex thread | SubharmoniqField no-audio fix and Field UI screenshot refinement | `src/DaisySubharmoniqCore.cpp`, `tests/test_subharmoniq_core.cpp`, `src/BoardControlMapping.cpp`, `tests/test_board_control_mapping.cpp`, `src/BoardProfile.cpp`, `tests/test_board_profile.cpp`, `src/juce/DaisyHostPluginEditor.*`, `src/juce/DaisyHostPluginProcessor.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `../field/SubharmoniqField/README.md`, `../field/SubharmoniqField/CONTROLS.md`, tracking docs | Red: `ctest --test-dir build -C Debug --output-on-failure -R "SubharmoniqCoreTest.PlayToggleRunsInternalClockAndProducesAudio"` failed with zero triggers and zero audio. Green: `cmd /c build_host.cmd` first compiled but failed old SW utility-label expectations; targeted rerun for `RenderRuntimeTest.PolyOscFieldSurfaceMapsK5ToWaveformOnly`, `RenderRuntimeTest.FieldExtendedSurfaceStateMirrorsOutputsSwitchesAndLeds`, and `DaisyHostRenderSmoke` passed `3/3`. Firmware: `make` from `field/SubharmoniqField` passed with FLASH `124424 B` / `94.93%`. QAE: `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`. Final wrapper gate `cmd /c build_host.cmd` passed with Release `ctest` `175/175`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `field/SubharmoniqField/README.md`, `field/SubharmoniqField/CONTROLS.md` | No code blocker after implementation. UI screenshot capture was not available through the automated smoke path; standalone/render smoke passed, but hands-on visual inspection and hardware flashing remain pending. | SubharmoniqField now has a build-tested internal clock path, so B7 play can produce audio without external clock. The host Field surface now reflects SW1/SW2 navigation, X/C virtual switch shortcuts, knob parameter labels, octave-visible keyboard range, and top-grouped audio/gate/MIDI artwork. |
| 2026-04-26 | Local Codex thread | Subharmoniq portable core, hosted app, and Field firmware adapter | `include/daisyhost/DaisySubharmoniqCore.h`, `src/DaisySubharmoniqCore.cpp`, `include/daisyhost/apps/SubharmoniqCore.h`, `src/apps/SubharmoniqCore.cpp`, `src/AppRegistry.cpp`, `CMakeLists.txt`, `tests/test_subharmoniq_core.cpp`, `tests/test_app_registry.cpp`, `../field/SubharmoniqField/*`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed first because `src/DaisySubharmoniqCore.cpp` did not exist. Green targeted: Debug `unit_tests` built; `ctest --test-dir build -C Debug --output-on-failure -R "(SubharmoniqCoreTest|AppRegistryTest|BoardControlMappingTest)"` passed `20/20`. Firmware: `make` from `field/SubharmoniqField` first failed on const `MidiEvent::AsNoteOn()` use, then passed after the adapter fix with FLASH `124176 B` / `94.74%`. QAE: `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .` passed with `0 error(s), 0 warning(s)`. A later direct Debug host build hit the known duplicate `Path` / `PATH` MSBuild issue; wrapper gate `cmd /c build_host.cmd` passed with Release `ctest` `173/173`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `FIELD_PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, Field examples, `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md` | No code blocker after implementation. `field/SubharmoniqField` is build/QAE-verified only; `make program`, audio output validation, SW1/SW2 pseudo-encoder hands-on validation, CV voltage checks, MIDI hardware checks, and DAW/VST3 manual validation were not run. | Subharmoniq Round 1 landed as a portable DaisyHost core, first-class hosted app, and Field firmware adapter. Next safe step is hardware flashing plus the `CONTROLS.md` manual checklist; Round 2 filter switching remains intentionally deferred. |
| 2026-04-25 | Local Codex thread | PolyOsc hosted-app import | `include/daisyhost/DaisyPolyOscCore.h`, `src/DaisyPolyOscCore.cpp`, `include/daisyhost/apps/PolyOscCore.h`, `src/apps/PolyOscCore.cpp`, `src/AppRegistry.cpp`, `CMakeLists.txt`, `tests/test_daisy_polyosc_core.cpp`, `tests/test_polyosc_core.cpp`, `tests/test_app_registry.cpp`, `tests/test_board_control_mapping.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/polyosc_smoke.json`, `training/examples/field_polyosc_surface_smoke.json`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` failed on missing `daisyhost/DaisyPolyOscCore.h` and `daisyhost/apps/PolyOscCore.h`. Green: `cmake --build build --config Debug --target unit_tests` passed; focused Debug payload passed `11/11`; raw Release `cmake --build build --config Release --target DaisyHostRender` first hit the known duplicate `Path` / `PATH` issue; sanitized Release `DaisyHostRender` build passed; render smoke passed including PolyOsc and Field PolyOsc scenarios; `cmd /c build_host.cmd` passed with Release `ctest` `168/168`. | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `patch/PolyOsc/README.md`, `patch/PolyOsc/PolyOsc.cpp`, hosted-app/core/render/Field mapping source | No code blocker. Raw MSBuild still needs the wrapper or sanitized one-path environment. No firmware `make` was required because this pass stayed host-side; Field firmware, hardware voltage validation, mixed-board racks, and DAW/VST3 manual validation remain deferred. | PolyOsc is now a first-class DaisyHost hosted app. Manager-readable result: Patch PolyOsc behavior is available in DaisyHost with Patch K1-K4/encoder controls, mixed host output from out 4, and host-side Field K5 waveform mapping without adding real Field firmware. |
| 2026-04-25 | Local Codex thread + Lane B worker | Field ergonomics polish and board-generic UI cleanup | `include/daisyhost/HostedAppCore.h`, `src/BoardControlMapping.cpp`, `src/BoardProfile.cpp`, `src/apps/MultiDelayCore.cpp`, `src/apps/TorusCore.cpp`, `src/apps/CloudSeedCore.cpp`, `src/apps/BraidsCore.cpp`, `src/apps/HarmoniqsCore.cpp`, `src/apps/VASynthCore.cpp`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_board_control_mapping.cpp`, `tests/test_board_profile.cpp`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` first failed on missing `HostedAppPatchBindings::knobParameterIds`. Red Lane B: `ctest --test-dir build -C Debug --output-on-failure -R "BoardProfileTest"` failed `10/11` because Field surface controls had empty target ids. Green: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|RenderRuntimeTest|HostSessionStateTest|LiveRackTopologyTest)"` passed `70/70`; raw Release `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone` first hit the known duplicate `Path` / `PATH` MSBuild issue, then the sanitized rerun passed; standalone smoke passed for `daisy_patch` and `daisy_field`; render smoke passed all scenarios including Field native/extended/selected-node surface scenarios; `cmd /c build_host.cmd` passed with Release `ctest` `159/159`. | `FIELD_PROJECT_TRACKER.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, relevant Field/app/profile/editor source | No code blocker. Raw MSBuild still needs the wrapper or a sanitized one-path environment in this shell. Manual DAW/VST3 validation, Field firmware, real hardware voltage output, mixed-board racks, routing changes, and DAW automation expansion remain deferred. | F1/F2 landed. Manager-readable result: Field's extra knobs now expose more useful app controls by excluding explicit K1-K4 targets, and the Field editor relies more on board-profile target metadata instead of hardcoded Field/Patch naming. This improves Field ergonomics and lowers future board-support cost while keeping `daisy_patch` default and the rack topology frozen. |
| 2026-04-25 | Local Codex thread | Board/app-independent knob-drag crash fix | `src/juce/DaisyHostPluginProcessor.h`, `src/juce/DaisyHostPluginProcessor.cpp`, tracking docs | Root-cause inspection found message-thread knob updates and timer/readback paths could call into the selected hosted-app core while the audio thread was also processing the same core. Fix: serialize hosted-core access and selected-node/core pointer handoff with `coreStateMutex_`. Checks: `cmake --build build --config Debug --target unit_tests` passed; raw `cmake --build build --config Debug --target DaisyHostPatch_Standalone` first hit the known duplicate `Path` / `PATH` MSBuild issue; sanitized Debug standalone build passed; `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"` passed `66/66`; visible Debug standalone manual validation launched `--board daisy_field --app multidelay`, survived Field knob dragging, then launched `--board daisy_patch --app multidelay` and survived Patch knob dragging; sanitized Release build for `DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone` passed; standalone smoke passed for `daisy_patch` and `daisy_field`; render smoke passed; `cmd /c build_host.cmd` passed with Release `ctest` `155/155`. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, processor/editor source paths involved in knob/control flow | No code blocker. A red automated test was not added because the failure is a JUCE audio/message-thread race outside the current unit harness; coverage is targeted build/test plus visible Patch and Field standalone knob-drag validation. Manual DAW/VST3 load validation remains deferred. | Crash fix landed. Manager-readable result: knob movement no longer lets the UI thread and audio thread mutate/read the same hosted app core at the same time, which explains and addresses the all-app/all-board crash without changing `daisy_patch` default, rack topology, or Field/Patch control semantics. |
| 2026-04-25 | Local Codex thread | Field project tracker creation and sprint backlog organization | `FIELD_PROJECT_TRACKER.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `SKILL_PLAYBOOK.md` | Docs-only checks: `rg -n "Field|daisy_field|firmware|hardware|DAW|mixed-board|ergonomics|deferred|next" README.md CHECKPOINT.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHANGELOG.md`; `rg -n "FIELD_PROJECT_TRACKER|Field DAW|Sprint F|155/155|daisy_field" FIELD_PROJECT_TRACKER.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md SKILL_PLAYBOOK.md`; `git diff --check -- FIELD_PROJECT_TRACKER.md PROJECT_TRACKER.md WORKSTREAM_TRACKER.md SKILL_PLAYBOOK.md`. No build was run because this was a docs-only tracker creation. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | No code blocker. DAW/VST3 validation remains an explicit todo outside the implementation backlog. | Field follow-on work now has a dedicated tracker. Manager-readable result: implementation options are organized into sprints, and manual DAW/VST3 validation is tracked separately so it cannot be confused with new implementation scope. |
| 2026-04-25 | Local Codex thread | Field selection, node evidence, and status cleanup | `src/HubSupport.cpp`, `src/RenderRuntime.cpp`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_hub_support.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/field_node_target_surface_smoke.json`, tracking docs | Red: `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"` first failed on Hub play plans still using command-line args only and Field surface manifests still reporting `node0` for a selected `node1` scenario. Green targeted: `cmake --build build --config Debug --target unit_tests` passed; `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"` passed `39/39`; processor snapshot/helper follow-up passed `42/42`; broader targeted Debug subset passed `77/77`. Release: raw `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_Standalone` first hit the known duplicate `Path` / `PATH` shell issue, then the sanitized rerun passed. Smoke: standalone passed for `daisy_patch` and `daisy_field`; render smoke passed including the new selected-node Field surface scenario. Full gate: `cmd /c build_host.cmd` passed with Release `ctest` `155/155`. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `training/README.md`, relevant Hub/render/editor/test source | No code blocker. Raw MSBuild still needs the checked-in wrapper or a sanitized one-path environment in this shell. Manual DAW/VST3 validation, Field firmware, real hardware voltage output, mixed-board racks, and deeper Field-specific app ergonomics remain deferred. | Field refinement closeout is landed. Manager-readable result: Field is not just selectable and rendered; Hub launch planning now uses the supported startup-request path, render/smoke proves selected-node Field surface evidence, and the UI/docs no longer describe Field host controls as future or Patch-only. |
| 2026-04-25 | Local Codex thread | Hub executable icon polish | `CMakeLists.txt`, `CHANGELOG.md`, `PROJECT_TRACKER.md` | `cmake --build build --config Release --target DaisyHostHub`: passed; generated resource check `Select-String build\\DaisyHostHub_artefacts\\JuceLibraryCode\\DaisyHostHub_resources.rc -Pattern "ICON|daisyhost|IDI"` confirmed `IDI_ICON1` and `IDI_ICON2` use generated `icon.ico`; `Get-Item build\\DaisyHostHub_artefacts\\Release\\DaisyHost Hub.exe` confirmed rebuilt executable exists. | `PROJECT_TRACKER.md`, `CHANGELOG.md`, CMake target definition | No blocker. This was a Hub-only resource polish change; no processor/render/runtime behavior changed. | `DaisyHostHub` now reuses the existing DaisyHost Patch compact icon asset through JUCE `ICON_SMALL`, so the Hub executable has a Windows icon after rebuild. |
| 2026-04-25 | Local Codex thread | Field extended host surface support | `BoardProfile`, `BoardControlMapping`, processor/editor Field surface APIs, effective snapshot, render manifest/runtime, smoke harness, `training/examples/field_extended_surface_smoke.json`, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` first failed on missing Field output/switch/LED bindings, indicator metadata, snapshot, and render manifest fields; green targeted: `cmake --build build --config Debug --target unit_tests` passed and `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|EffectiveHostStateSnapshotTest|RenderRuntimeTest|LiveRackTopologyTest)"` passed `68/68`; Release build: `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone` passed after normalizing duplicate `Path` / `PATH` for that raw subprocess; smoke: `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120` passed for `daisy_patch` and `daisy_field`; smoke: `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120` passed, including Field extended surface evidence; full gate: `cmd /c build_host.cmd` passed with Release `ctest` `152/152`. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, relevant board/profile/session/render/editor source | No code blocker after implementation. Raw `cmake --build` in this shell can still inherit duplicate `Path` / `PATH`; the checked-in `build_host.cmd` wrapper handled the full gate. Manual DAW/VST3 load validation, Field firmware, real hardware voltage output, mixed-board racks, and Field-specific app ergonomics remain deferred. | Field extended host surface support is landed. Manager-readable result: `daisy_field` is now automatically tested as a functioning host-side Field board with K1-K8, CV1-CV4, Gate In, A1-B8, CV OUT 1-2 monitor outputs, SW1/SW2 utility triggers, and LED evidence while `daisy_patch` remains default and the rack topology stays frozen. |
| 2026-04-25 | Local Codex thread | Daisy Field native controls implementation | `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp`, `CMakeLists.txt`, processor/editor Field surface APIs, render `surface_control_set`, Field native smoke scenario, tracking docs | Red: `cmake --build build --config Debug --target unit_tests` first failed on missing `daisyhost/BoardControlMapping.h`; red: a later Debug build failed on missing `RenderTimelineEventType::kSurfaceControlSet` / `RenderTimelineEvent::controlId`; green targeted: `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"` passed `57/57`; Release smoke: standalone and render `tests/run_smoke.py` passed, including Field shell and Field native controls scenarios; full gate: `cmd /c build_host.cmd` passed with Release `ctest` `144/144`. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `WORKSTREAM_TRACKER.md`, relevant board/profile/session/render/editor source | No code blocker after implementation; manual DAW/VST3 load validation remains deferred by existing policy, and no firmware `make` rerun was required because this package stayed host-side. | Field native controls landed on top of the existing `daisy_field` shell. At that checkpoint K1-K8/CV1-CV4/Gate In/A1-B8 were host-side mappings only; later same-day extended-surface work implemented host-side CV OUT monitor outputs, SW1/SW2, and LEDs. Field firmware, real hardware voltage output, Field-specific app ergonomics, DAW validation, and mixed-board racks remain follow-on work. |
| 2026-04-24 | Local Codex thread | Daisy Field board-support shell implementation | `BoardProfile`, `HubSupport`, board-id-safe processor/session/render paths, passive board panel metadata, smoke harness, tracking docs | Preflight: `git status --short --branch -- . ..\docs\plans\2026-04-24-daisyhost-field-board.md`; baseline `cmake --build build --config Debug --target unit_tests` (passed); red `cmake --build build --config Debug --target unit_tests` first failed on active Field profile contracts before the missing Field control kinds/profile implementation were restored; red `ctest --test-dir build -C Debug --output-on-failure -R "HubSupportTest.GeneratesFieldRenderScenarioInsteadOfReusingDefaultBoardExample"` failed before Hub generated a Field-specific render scenario; green `cmake --build build --config Debug --target unit_tests` (passed); green `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|HubSupportTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"` (`54/54` passed); full gate `cmd /c build_host.cmd` passed with Release `ctest` `136/136`; docs consistency: `git diff --check -- ...` passed, trailing-whitespace `Select-String ... -Pattern '[ \\t]+$'` returned no matches, and Field wording audit was reviewed. | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `WORKSTREAM_TRACKER.md`, `../docs/plans/2026-04-24-daisyhost-field-board.md`, `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `src/HubSupport.cpp`, `src/RenderRuntime.cpp`, `src/juce/DaisyHostPluginProcessor.*`, `src/juce/DaisyHostPluginEditor.*`, `tests/run_smoke.py`, `tests/test_hub_support.cpp`, `training/examples/field_cloudseed_shell_smoke.json` | No code blocker after implementation; manual DAW/VST3 load validation remains deferred by existing project policy, and no firmware `make` rerun was required because this package stayed host-side. | Field board-support shell landed through the existing factory seam with `daisy_patch` still default. `daisy_field` was carried by Hub/session/standalone/render/smoke paths and drawn as passive board metadata only; Field native controls were intentionally deferred until the 2026-04-25 package. |
| 2026-04-24 | Local Codex thread | Post-WS7 tracker split: dedicated workstream tracker + mirrored portfolio | `WORKSTREAM_TRACKER.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `README.md`, `CHANGELOG.md` | `ctest --test-dir build -C Release --output-on-failure -R "AppRegistryTest"` (`2/2` passed); `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 60` (passed); `cmd /c build_host.cmd` (passed, `128/128`); docs consistency: `rg -n "WORKSTREAM_TRACKER|128/128|implementation ready|Daisy Field" PROJECT_TRACKER.md CHECKPOINT.md README.md CHANGELOG.md`; `git diff --check -- PROJECT_TRACKER.md WORKSTREAM_TRACKER.md CHECKPOINT.md README.md CHANGELOG.md` | `PROJECT_TRACKER.md`, `WORKSTREAM_TRACKER.md`, `CHECKPOINT.md`, `README.md`, `CHANGELOG.md` | No new code blocker in this iteration; external DAW/VST3 validation remains intentionally deferred. | Dedicated workstream tracker now exists, the same portfolio is mirrored in `PROJECT_TRACKER.md`, the host gate is recorded green at `128/128`, and Daisy Field readiness is explicitly unlocked. Next safe starting point is the first parallel wave: `WS8`, `WS9`, `TF8`, `TF9`, and `TF12`. |
| 2026-04-24 | Local Codex thread | Daisy Field plan refinement: frozen rack + board-support package | `../docs/plans/2026-04-24-daisyhost-field-board.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Docs-only consistency checks: reread the Field plan, `PROJECT_TRACKER.md`, and `SKILL_PLAYBOOK.md`; `rg -n "rack|Field native|Milestone|board-support|daisy_field|factory seam|rack as frozen" ../docs/plans/2026-04-24-daisyhost-field-board.md PROJECT_TRACKER.md SKILL_PLAYBOOK.md`; `git diff --check -- ../docs/plans/2026-04-24-daisyhost-field-board.md PROJECT_TRACKER.md SKILL_PLAYBOOK.md`; `Select-String -Path ../docs/plans/2026-04-24-daisyhost-field-board.md,PROJECT_TRACKER.md,SKILL_PLAYBOOK.md -Pattern '[ \\t]+$'` | `../docs/plans/2026-04-24-daisyhost-field-board.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | No code/runtime test was run because this was a docs-only refinement. Full Release aggregate gate was subject to the known machine-level unit-test payload lock at that time, but the refined plan no longer blocked starting the narrow board-support shell package on that gate. | Plan followed the requested decision: freeze the rack and start Field as a separate board-support package through the existing board factory seam. Field native controls were deferred to the later 2026-04-25 package. |
| 2026-04-24 | Local Codex thread | Daisy Field implementability verification and draft implementation plan | `../docs/plans/2026-04-24-daisyhost-field-board.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Docs-only consistency checks: read `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `tests/test_board_profile.cpp`, `src/HubSupport.cpp`, `tests/test_hub_support.cpp`, `include/daisyhost/HostedAppCore.h`, `src/juce/DaisyHostPluginProcessor.*`, `src/juce/DaisyHostPluginEditor.*`, `../libDaisy/src/daisy_field.h`, `../field/README.md`; `git diff --check -- ../docs/plans/2026-04-24-daisyhost-field-board.md PROJECT_TRACKER.md SKILL_PLAYBOOK.md`; `Select-String -Path ../docs/plans/2026-04-24-daisyhost-field-board.md,PROJECT_TRACKER.md,SKILL_PLAYBOOK.md -Pattern '[ \\t]+$'` | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md`, `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `tests/test_board_profile.cpp`, `src/HubSupport.cpp`, `tests/test_hub_support.cpp`, `include/daisyhost/HostedAppCore.h`, `../libDaisy/src/daisy_field.h`, `../field/README.md` | No code/runtime test was run because this was a docs-only planning pass. At that historical point, Field implementation was blocked behind the full Release host-gate issue until an admin-capable or lock-free run cleared `cmd /c build_host.cmd`; that blocker was later cleared. | Field was judged implementable but not ready to start in that checkpoint: the board factory/session/hub seams existed, while live controls/editor behavior was still Patch-shaped. The later 2026-04-24 shell package and 2026-04-25 native-controls package supersede this planning state. |
| 2026-04-24 | Local Codex thread | Additional hosted apps: `harmoniqs` + `vasynth` | `include/daisyhost/DaisyHarmoniqsCore.h`, `src/DaisyHarmoniqsCore.cpp`, `include/daisyhost/apps/HarmoniqsCore.h`, `src/apps/HarmoniqsCore.cpp`, `include/daisyhost/DaisyVASynthCore.h`, `src/DaisyVASynthCore.cpp`, `include/daisyhost/apps/VASynthCore.h`, `src/apps/VASynthCore.cpp`, `src/AppRegistry.cpp`, `CMakeLists.txt`, `tests/test_app_registry.cpp`, `tests/test_daisy_harmoniqs_core.cpp`, `tests/test_harmoniqs_core.cpp`, `tests/test_daisy_vasynth_core.cpp`, `tests/test_vasynth_core.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/harmoniqs_smoke.json`, `training/examples/vasynth_smoke.json`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `README.md`, `CHANGELOG.md` | Red: `ctest --test-dir build -C Debug --output-on-failure -R "AppRegistryTest\\.RegistersMultiDelayTorusCloudSeedBraidsHarmoniqsAndVASynthApps"` (failed first because the registry still exposed only four apps); after adding the new tests to `CMakeLists.txt`, `cmake --build build --config Debug --target unit_tests` failed first on the missing Harmoniqs/VA Synth wrapper and app headers. Green targeted: `cmake --build build --config Debug --target unit_tests` (passed); direct payload run `py -3 tests/run_unit_test_payload.py <fresh Debug payload> --gtest_filter="AppRegistryTest.*:DaisyHarmoniqsCoreTest.*:HarmoniqsCoreTest.*:DaisyVASynthCoreTest.*:VASynthCoreTest.*:RenderRuntimeTest.ProducesDeterministicOfflineRenderForHarmoniqs:RenderRuntimeTest.ProducesDeterministicOfflineRenderForVASynth"` (passed, `18/18`); `cmake --build build --config Release --target DaisyHostRender` (passed); `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120` (passed). | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHANGELOG.md` | Direct `ctest -C Debug -R ...` can still point at a stale unbuilt run-tag payload in this checkout, and the broader Release `ctest` gate remains blocked by the machine-level lock/read issue on the freshly linked Release payload. No firmware-facing code changed, so no `make` rerun was required. | `harmoniqs` and `vasynth` are now source-backed, targeted-test-backed, and Release-render-smoke-backed from this checkout. Next safe starting point is either a full Release host-gate rerun once the payload lock is cleared or the next hosted-app wave on the same portable-core plus adapter pattern. |
| 2026-04-24 | Local Codex thread | Rack freeze blocker isolation and wrapper mitigation | `CMakeLists.txt`, `build_host.ps1`, `tests/run_unit_test_payload.py`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `SKILL_PLAYBOOK.md`, `README.md`, `CHANGELOG.md` | Root-cause diagnostics: exact-name writes in `build\\test_bin\\Release` (`DaisyHostUnitTestsRunner.exe` denied; `DaisyHostUnitTests.exe`, `unit_tests.exe`, and other nearby names allowed); scratch-directory check showed `DaisyHostUnitTests.exe` was writable/runnable outside the original path; `cmake --build build --config Release --target unit_tests` after renaming/output-path changes first passed link, then later re-failed on stale locked outputs; `cmd /c build_host.cmd` after the wrapper mitigation rebuilt `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone`, then `ctest -C Release` ran and passed `DaisyHostStandaloneSmoke` plus `DaisyHostRenderSmoke` but failed `110/112` tests because `tests/run_unit_test_payload.py` received `PermissionError` when reading `build\\unit_test_bin\\<run-tag>\\Release\\DaisyHostTestPayload.bin`; attempted `Add-MpPreference -ExclusionPath ...\\build\\unit_test_bin,...\\build\\unit_test_run` still failed due insufficient permissions. | `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `SKILL_PLAYBOOK.md`, `README.md`, `CHANGELOG.md` | The old link-time blocker is reduced, but the Release rack freeze gate is still blocked by a machine-level read/lock policy on the freshly linked Release unit-test payload itself. Daisy Field work remains blocked. | Current safe starting point is an admin-capable security exception or clean admin shell, then a fresh `.\build_host.cmd` rerun. Do not start Daisy Field implementation before that rerun is fully green. |
| 2026-04-24 | Local Codex thread | Rack freeze resume and Daisy Field gating check | `build_host.ps1`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `SKILL_PLAYBOOK.md` | Diagnostic: `Get-Process | Where-Object { $_.ProcessName -like '*DaisyHostUnitTests*' -or $_.ProcessName -like '*DaisyHost Patch*' }`; `.\\build_host.cmd` (failed first in `build_host.ps1` on duplicate `Path`/`PATH` env-provider lookup); `$p = Join-Path (Get-Location) 'build\\test_bin\\Release\\probe.exe'; [System.IO.File]::WriteAllBytes(...)` (passed); verified safe cleanup of `build\\test_bin\\Release` and `build\\unit_tests.dir\\Release`; `.\\build_host.cmd` (reached configure/build and rebuilt the Release target set, then `ctest -C Release` failed because `DaisyHostUnitTestsRunner.exe` was `resource busy or locked`); direct launch of `.\\build\\test_bin\\Release\\DaisyHostUnitTestsRunner.exe --gtest_filter=BoardProfileTest.BoardFactoryRejectsUnknownBoardsCleanly` (failed: file in use by another process); waited relaunch (still failed); copied and ran `.\\build\\Release\\DaisyHostRender.exe` from both `build\\Release` and `build\\test_bin\\Release` (passed, proving the directory itself is usable); attempted `Add-MpPreference -ExclusionPath ...\\DaisyHost\\build\\test_bin\\Release` (failed: insufficient permissions). | `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `SKILL_PLAYBOOK.md` | Rack freeze is still blocked by a machine-level lock/removal of the Release unit-test runner. The wrapper fix is real, but the canonical Release host gate is still not green, so Daisy Field work remains blocked. | Current safe starting point is machine-level remediation for the Release runner path, then a fresh `.\build_host.cmd` rerun. Do not start Daisy Field implementation before that rerun is fully green. |
| 2026-04-24 | Local Codex thread | Braids verification hardening + host wrapper resilience | `build_host.ps1`, `CMakeLists.txt`, `tests/test_render_runtime.cpp`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Failed/diagnostic: `.\build_host.cmd` inside the sandbox rebuilt Release targets but `ctest -C Release` could not launch `build/Release/DaisyHostUnitTestsRunner.exe` (`resource busy or locked`); elevated `cmd /c build_host.cmd` first failed because `build_host.ps1` assumed `Env:PATH` existed when only `Env:Path` was present; after fixing PATH normalization, elevated Release build reached CTest but still could not launch the Release runner; focused Debug run initially failed `RenderRuntimeTest.ProducesDeterministicOfflineRenderForBraids` because the test scheduled a `0.30s` gate event inside a `0.25s` render. Green: `cmake --build build --config Debug --target unit_tests` (passed); `ctest --test-dir build -C Debug --output-on-failure -R "(AppRegistryTest|BraidsCoreTest|DaisyBraidsCoreTest|RenderRuntimeTest)"` (`27/27` passed); `ctest --test-dir build -C Debug --output-on-failure -E "DaisyHost(Standalone|Render)Smoke"` (`110/110` passed); `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release --timeout-seconds 120` (passed). | `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `CHANGELOG.md`, `SKILL_PLAYBOOK.md` | Full Release `ctest` remains blocked because CTest cannot launch the Release `DaisyHostUnitTestsRunner.exe` path in this shell, even after moving the runner under `build/test_bin/<config>/`. External DAW/VST3 manual validation remains deferred. | Braids now has real unit-test execution evidence from the Debug runner plus current Release smoke evidence. The next safe starting point is diagnosing why only the Release unit-test executable cannot be launched by CTest; product/runtime app binaries and smoke scenarios are otherwise current. |
| 2026-04-23 | Local Codex thread | Workstream 6: additional hosted app pilot (`braids`) | `include/daisyhost/DaisyBraidsCore.h`, `src/DaisyBraidsCore.cpp`, `include/daisyhost/apps/BraidsCore.h`, `src/apps/BraidsCore.cpp`, `src/AppRegistry.cpp`, `tests/test_daisy_braids_core.cpp`, `tests/test_braids_core.cpp`, `tests/test_app_registry.cpp`, `tests/test_render_runtime.cpp`, `tests/run_smoke.py`, `training/examples/braids_smoke.json`, `include/stmlib/stmlib.h`, `include/stmlib/utils/dsp.h`, `CMakeLists.txt`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red: `cmake --build build --config Release --target unit_tests` (failed first on missing Braids wrapper/app surfaces before implementation). Green/build-backed: `cmake -S . -B build`; `cmake --build build --config Release --target unit_tests`; `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`; `ctest --test-dir build -C Release --output-on-failure -R "DaisyHost(Standalone|Render)Smoke"` (`2/2` passed); `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release --timeout-seconds 120` (passed). | `AGENTS.md`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | `DaisyHostUnitTestsRunner.exe` can still vanish or remain externally locked after link in this shell, so direct unit-test execution and the full host gate were not rerun against the final visible-rack plus `braids` code. | `braids` is now source-backed, build-backed, and smoke-backed from this checkout: DaisyHost ships a new portable `DaisyBraidsCore`, a percussion-first hosted app with `Drum` / `Finish` pages, MIDI-first triggering plus gate alias, checked-in `braids_smoke.json`, and DaisyHost-local `stmlib` host shims for the vendored Braids sources. Next safe starting point is restoring stable `unit_tests` executable output so Braids-targeted and full-gate `ctest` can be rerun cleanly. |
| 2026-04-23 | Local Codex thread | WS7 sprint: visible 2-node live rack + board-selection seam | `include/daisyhost/LiveRackTopology.h`, `src/LiveRackTopology.cpp`, `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp`, `include/daisyhost/HostSessionState.h`, `src/HostSessionState.cpp`, `include/daisyhost/RenderTypes.h`, `src/RenderRuntime.cpp`, `include/daisyhost/EffectiveHostStateSnapshot.h`, `src/EffectiveHostStateSnapshot.cpp`, `src/juce/DaisyHostPluginProcessor.h`, `src/juce/DaisyHostPluginProcessor.cpp`, `src/juce/DaisyHostPluginEditor.h`, `src/juce/DaisyHostPluginEditor.cpp`, `tests/test_live_rack_topology.cpp`, `tests/test_board_profile.cpp`, `tests/test_host_session_state.cpp`, `tests/test_render_runtime.cpp`, `tests/test_effective_host_state_snapshot.cpp`, `CMakeLists.txt`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red/diagnostic: `ctest --test-dir build -C Release --output-on-failure -R "(HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest|LiveRackTopologyTest|BoardProfileTest)"` (earlier rebuilt binary still failed `RenderRuntimeTest.RunsTwoNodeAudioChainScenario` before the final rerun path was rebuilt); rebuilt verification: `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake -S . -B build`; `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target unit_tests`; `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`; `ctest --test-dir build -C Release --output-on-failure -R "DaisyHost(Standalone|Render)Smoke"` (`2/2` passed); `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release` (passed); `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release` (passed); direct `DaisyHostRender.exe` runs against temporary `build/smoke/rack_forward_runtime.json` and `build/smoke/rack_reverse_runtime.json` (both passed and wrote manifests with the expected rack fields). | `AGENTS.md`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | External process lock on `build\\Release\\DaisyHostUnitTestsRunner.exe` / `DaisyHostUnitTestsRunner.exe` prevented executing the rebuilt unit-test binary, so the final full host gate for the visible-rack code has not yet been rerun in this shell. External DAW/VST3 manual validation also remains intentionally deferred until post-WS7. | Visible 2-node live rack plus board-selection seam is now source-backed, build-backed, and smoke-backed from this checkout. What this unlocks: DaisyHost can now run two hosted apps live in one plugin/standalone instance with explicit entry/output topology presets and selected-node editing. Next safe starting point is a fresh full host-gate rerun once the unit-test file lock clears, then either richer routing beyond serial audio-only presets or the deferred post-WS7 DAW validation pass. |
| 2026-04-23 | Local Codex thread | Preclaim: visible 2-node live rack MVP + board-selection seam | `AGENTS.md`, `PROJECT_TRACKER.md` | Docs-only preclaim checks: `Get-Content AGENTS.md -TotalCount 320`; `Get-Content PROJECT_TRACKER.md -TotalCount 340`; `Get-Content README.md -TotalCount 260`; `Get-Content CHECKPOINT.md -TotalCount 260`; `Get-Content src/juce/DaisyHostPluginProcessor.h -TotalCount 240`; `Get-Content src/juce/DaisyHostPluginProcessor.cpp -TotalCount 580`; `Get-Content src/juce/DaisyHostPluginEditor.h -TotalCount 260`; `Get-Content src/juce/DaisyHostPluginEditor.cpp -TotalCount 420`; `Get-Content include/daisyhost/HostSessionState.h -TotalCount 220`; `Get-Content src/HostSessionState.cpp -TotalCount 260`; `Get-Content include/daisyhost/RenderTypes.h -TotalCount 260`; `Get-Content include/daisyhost/EffectiveHostStateSnapshot.h -TotalCount 220`; `Get-Content src/HubSupport.cpp -TotalCount 220`; `git status --short --branch`; `git diff --check -- AGENTS.md PROJECT_TRACKER.md` | `AGENTS.md`, `README.md`, `CHECKPOINT.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Current runtime/editor still remain single-node, so this sprint must land runtime, UI, session/render, and board-seam work together to avoid a half-rack state. | Ownership for the new sprint is now explicit: Worker 1 owns the live-rack topology helper/tests, Worker 2 owns the rack header/topology visuals plus board-profile seam, the Main Integrator owns live multi-node processor/session/render/snapshot integration, and Worker 3 will close the docs after fresh verification. |
| 2026-04-23 | Local Codex thread | Tracker update: Daisy Field green-light milestone | `PROJECT_TRACKER.md` | Docs-only consistency checks: `Get-Content PROJECT_TRACKER.md -TotalCount 320`; `rg -n "Field|green-light|Visible rack editor|Deeper routing|board-selection seam" PROJECT_TRACKER.md`; `Get-Content include/daisyhost/BoardProfile.h -TotalCount 220`; `Get-Content src/BoardProfile.cpp -TotalCount 320`; `Get-Content tests/test_hub_support.cpp -TotalCount 220`; `Get-Content src/juce/DaisyHostPluginProcessor.h -TotalCount 240`; `Get-Content src/juce/DaisyHostPluginProcessor.cpp -TotalCount 320`; `git diff --check -- PROJECT_TRACKER.md` | `PROJECT_TRACKER.md`, `README.md`, `CHECKPOINT.md` | No code/runtime blocker for the tracker-only update; Daisy Field itself remains intentionally blocked behind the new green-light milestone. | Tracker now records the exact point when Daisy Field can start safely in parallel: after the visible 2-node live rack MVP is landed, stabilized, and board-selection seams are explicit. Next safe starting point remains the live rack/runtime productionization package, not Field implementation yet. |
| 2026-04-23 | Local Codex thread | Combined WS6/WS7 implementation: MetaControllers + rack-ready session/render contract | `include/daisyhost/HostedAppCore.h`, `include/daisyhost/HostSessionState.h`, `include/daisyhost/RenderTypes.h`, `include/daisyhost/EffectiveHostStateSnapshot.h`, `include/daisyhost/apps/MultiDelayCore.h`, `include/daisyhost/apps/CloudSeedCore.h`, `src/HostSessionState.cpp`, `src/RenderRuntime.cpp`, `src/EffectiveHostStateSnapshot.cpp`, `src/apps/MultiDelayCore.cpp`, `src/apps/CloudSeedCore.cpp`, `src/juce/DaisyHostPluginProcessor.h`, `src/juce/DaisyHostPluginProcessor.cpp`, `tests/test_multidelay_core.cpp`, `tests/test_cloudseed_core.cpp`, `tests/test_host_session_state.cpp`, `tests/test_render_runtime.cpp`, `tests/test_effective_host_state_snapshot.cpp`, `README.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | Red before integration was already recorded in the current implementation session via the sanitized `cmake --build build --config Release --target unit_tests` failure for the missing session/render/snapshot fields. Green targeted: `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target unit_tests; ctest --test-dir build -C Release --output-on-failure -R "(MultiDelayCoreTest|CloudSeedCoreTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"` (`44/44` passed). Full gate: `cmd /c build_host.cmd` (`89/89` passed). | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | No project blocker after implementation; the remaining intentional gap is still the deferred post-WS7 external DAW/VST3 manual validation. | WS6 is now landed and WS7 groundwork is now source-backed from this checkout. What this unlocks: DaisyHost now has high-level semantic controls for `multidelay` and `cloudseed`, and its save/load plus offline render contracts can already represent multiple nodes and routes without forcing a visible rack UI rewrite first. Next safe starting point is visible rack/runtime productionization or an external readback/export surface. |
| 2026-04-23 | Worker 3 / Local Codex thread | Combined WS6/WS7 preclaim: `MetaControllers` + rack-ready session/render contract | `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md` | Docs-only consistency checks: `Get-Content README.md -TotalCount 260`; `Get-Content CHECKPOINT.md -TotalCount 260`; `Get-Content PROJECT_TRACKER.md -TotalCount 320`; `Get-Content SKILL_PLAYBOOK.md -TotalCount 320`; `Get-Content CHANGELOG.md -TotalCount 220`; `rg -n "WS6|WS7|Workstream 6|Workstream 7|MetaController|rack-ready|session/render contract" PROJECT_TRACKER.md SKILL_PLAYBOOK.md CHECKPOINT.md README.md CHANGELOG.md AGENTS.md`. No build/test commands were run in this preclaim-only step. | `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`, `CHANGELOG.md` | No blocker at claim time; implementation and verification evidence remain pending in the code-owning slices. | Combined WS6/WS7 package is now reserved in the tracker as pre-implementation work only, and expected skill use is predeclared in `SKILL_PLAYBOOK.md`. Do not treat `MetaControllers` or rack-ready session/render behavior as landed or verified until later ledger rows add red/green evidence. |
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
| Hosted app registry | Registers available DaisyHost apps and resolves a default app when an unknown id is requested. | `src/AppRegistry.cpp` registers `multidelay`, `torus`, `cloudseed`, `braids`, `harmoniqs`, `vasynth`, and `polyosc`; `tests/test_app_registry.cpp` covers registration and unknown-id fallback. |
| Patch board profile | Models the virtual Daisy Patch surface, control hierarchy, ports, and panel artwork metadata. | `src/BoardProfile.cpp` builds the profile; `tests/test_board_profile.cpp` checks port counts, Patch-like placement, artwork markers, and the final `CTRL 1..4 + ENC` mapping. |
| MultiDelay shared core/menu model | Exposes canonical parameters, OLED/menu interaction, Patch bindings, and deterministic DSP state for the regression fixture app. | `src/apps/MultiDelayCore.cpp` plus `tests/test_multidelay_core.cpp` and `tests/test_parameter_parity.cpp` cover menu editing, `last touch wins`, control mapping, state capture/restore, and menu-vs-direct parameter parity. |
| Torus hosted app | Provides a second hosted app with stable metadata, display/menu naming, and deterministic render behavior. | `src/apps/TorusCore.cpp`; `tests/test_torus_core.cpp` covers metadata/menu, deterministic reset/render, silent boot without excitation, and display compaction rules. |
| CloudSeed hosted app | Hosts a portable stereo reverb wrapper with performance-page knob remapping, named MetaControllers, utility actions, deterministic render state, and a parameter-based arpeggiator. | `src/DaisyCloudSeedCore.cpp` owns the portable CloudSeed mapping/state layer and deterministic arp effective-state stepping; `src/apps/CloudSeedCore.cpp` exposes the DaisyHost adapter, macro/arp/menu/display model, and page behavior; `tests/test_daisy_cloudseed_core.cpp`, `tests/test_cloudseed_core.cpp`, `tests/test_render_runtime.cpp`, and `training/examples/cloudseed_smoke.json` cover the shared core, hosted app contract, render flow, arp path, and smoke scenario. |
| Braids hosted app | Hosts a portable percussion-first Braids wrapper with page-remapped knobs, MIDI-first triggering, gate alias support, and deterministic render state. | `src/DaisyBraidsCore.cpp` owns the portable Braids wrapper, model subset, and trigger/state layer; `src/apps/BraidsCore.cpp` exposes the DaisyHost adapter, page/menu/display model, and strike utilities; `tests/test_daisy_braids_core.cpp`, `tests/test_braids_core.cpp`, and `training/examples/braids_smoke.json` cover the shared core, hosted app contract, and smoke path. |
| Harmoniqs hosted app | Hosts a portable additive Harmoniqs wrapper with page-remapped knobs, MIDI-first triggering, gate alias support, and deterministic render state. | `src/DaisyHarmoniqsCore.cpp` owns the portable Harmoniqs wrapper, harmonic-state layer, and trigger/state logic; `src/apps/HarmoniqsCore.cpp` exposes the DaisyHost adapter, page/menu/display model, and utility actions; `tests/test_daisy_harmoniqs_core.cpp`, `tests/test_harmoniqs_core.cpp`, `tests/test_render_runtime.cpp`, and `training/examples/harmoniqs_smoke.json` cover the shared core, hosted app contract, deterministic render path, and smoke scenario. |
| VA Synth hosted app | Hosts a portable seven-voice subtractive synth wrapper with page-remapped knobs, MIDI-first polyphonic triggering, gate alias support, and deterministic render state. | `src/DaisyVASynthCore.cpp` owns the portable polyphonic voice/state layer and canonical parameter contract; `src/apps/VASynthCore.cpp` exposes the DaisyHost adapter, page/menu/display model, and utility actions; `tests/test_daisy_vasynth_core.cpp`, `tests/test_vasynth_core.cpp`, `tests/test_render_runtime.cpp`, and `training/examples/vasynth_smoke.json` cover the shared core, hosted app contract, deterministic render path, and smoke scenario. |
| PolyOsc hosted app | Hosts a portable Patch PolyOsc wrapper with three oscillator outputs, mixed output 4, Patch frequency/global/waveform controls, and host-side Field K5 waveform mapping. | `src/DaisyPolyOscCore.cpp` owns the portable oscillator/state layer; `src/apps/PolyOscCore.cpp` exposes the DaisyHost adapter, menu/display model, Patch bindings, and output-port behavior; `tests/test_daisy_polyosc_core.cpp`, `tests/test_polyosc_core.cpp`, `tests/test_render_runtime.cpp`, `training/examples/polyosc_smoke.json`, and `training/examples/field_polyosc_surface_smoke.json` cover the shared core, hosted app contract, deterministic render path, and Field surface smoke path. |
| MetaController / macro layer | Exposes deterministic semantic controls that map one named macro to multiple canonical parameters without adding a second saved state layer. | `HostedAppCore` now exposes `GetMetaControllers()`, `SetMetaControllerValue(...)`, and `GetMetaControllerValue(...)`; `src/apps/MultiDelayCore.cpp` and `src/apps/CloudSeedCore.cpp` implement the macro profiles; `tests/test_multidelay_core.cpp` and `tests/test_cloudseed_core.cpp` cover exposure, write-through, and menu-driven updates. |
| Live rack topology helper | Canonicalizes the four visible audio-only rack topology presets and validates which route shapes are legal in the current two-node audio contract. | `src/LiveRackTopology.cpp`; `tests/test_live_rack_topology.cpp` covers preset expansion, reverse inference, unknown-node rejection, non-audio rejection, partial stereo rejection, and current-contract shape validation. |
| Host session persistence | Persists board choice, selected node, rack entry/output topology, app selection, canonical parameter values, MIDI learn bindings, host-side generator settings, plus node metadata and routes across save/load. | `src/HostSessionState.cpp`; `tests/test_host_session_state.cpp` now covers legacy v1/v2/v3/v4 compatibility plus v5 rack-global round trips and synthesized `node0` / rack defaults for older sessions. |
| Headless render runtime | Loads scenario JSON, validates event timelines and routes, renders offline audio, and writes manifest output for single-node and two-node serial rack cases. | `tools/render_app.cpp` and `src/RenderRuntime.cpp`; `tests/test_render_runtime.cpp` plus the later 2026-04-23 direct `DaisyHostRender.exe` proofs cover legacy parsing/validation, deterministic single-app render, node-targeted non-signal events, forward and reverse two-node audio chains, rejection of unsupported cross-node CV/gate/MIDI routes, and `audio.wav` + `manifest.json` output. |
| Automated standalone/render smoke harness | Launches the real standalone app for startup-stability smoke and exercises the real render CLI against checked-in smoke scenarios. | `tests/run_smoke.py` runs direct-entrypoint smoke checks; CTest wires it in as `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`, including repeated `multidelay` checksum comparison plus checked-in `torus`, `cloudseed`, `braids`, `harmoniqs`, `vasynth`, Field shell, and Field native controls render scenarios. |
| DAW-visible canonical parameter bridge | Exposes a fixed five-slot JUCE/VST automation bank with stable ids and active-app rebinding by canonical parameter metadata. | `src/HostAutomationBridge.cpp` ranks automatable parameters into `daisyhost.slot1`..`daisyhost.slot5`; `src/juce/DaisyHostPluginProcessor.cpp` owns the five JUCE parameters, app-switch/session-restoration rebinding, and DAW-write application back into canonical app parameters. |
| Effective-state inspection/readback API | Builds a processor-side snapshot of live canonical parameters, mapped automation slots, selected-node identity, node count, board/topology fields, active-node MetaControllers, node summaries, routes, CV state, gate state, and current audio-input configuration. | `src/EffectiveHostStateSnapshot.cpp` assembles the shared snapshot model; `DaisyHostPatchAudioProcessor::GetEffectiveHostStateSnapshot()` exposes it from live processor state. |
| Visible two-node live rack runtime | Runs two hosted apps live in one plugin/standalone instance and routes audio according to four operator-facing topology presets while keeping all controls selected-node scoped. | `src/juce/DaisyHostPluginProcessor.cpp` owns the two-node runtime, selected-node targeting, DAW-slot retargeting, and audio routing; `src/juce/DaisyHostPluginEditor.cpp` exposes the visible rack header, topology selector, role labels, and selected-node app inspection/editing. |
| Board factory seam | Creates board profiles by `boardId` instead of hardcoding Patch construction inside the live processor. | `src/BoardProfile.cpp` now exposes `TryCreateBoardProfile(...)` / `CreateBoardProfile(...)`; `tests/test_board_profile.cpp` covers successful `daisy_patch`, `daisy_field`, and unknown-board behavior. |
| Field native controls | Maps the Field surface to the selected node through existing host parameter, CV/gate, and MIDI paths while keeping the rack frozen. | `src/BoardControlMapping.cpp` builds K1-K8, CV1-CV4, Gate In, and A1-B8 bindings; `src/juce/DaisyHostPluginProcessor.cpp` applies the mappings; `src/juce/DaisyHostPluginEditor.cpp` renders interactive Field controls; `src/RenderRuntime.cpp` applies `surface_control_set` events for render scenarios. |
| Dataset orchestration and hub launch planning | Expands dataset jobs and prepares `Play / Test`, `Render`, and `Train` launch plans around the same render/training entrypoints. | `training/render_dataset.py`, `src/HubSupport.cpp`, and `src/hub/*`; `tests/test_hub_support.cpp` covers launch planning, startup handoff, profile persistence, generated scenario/job creation, and tool-path discovery. |
| MIDI interaction path | Supports computer-keyboard note entry, MIDI event tracking, note preview, and CC-learn plumbing in the host processor. | `src/ComputerKeyboardMidi.cpp`, `src/MidiEventTracker.cpp`, `src/MidiNotePreview.cpp`, and `src/juce/DaisyHostPluginProcessor.cpp`; tests cover keyboard mapping and tracker formatting while processor code owns learn/binding state. |
| Version/About surfacing | Exposes the current version, build identity, and release highlights to the host UI/About flow. | `project(DaisyHost VERSION 0.2.0 ...)` in `CMakeLists.txt`, `src/VersionInfo.cpp`, and `tests/test_version_info.cpp`. |

## Refinements Worth Doing

| Function | What it should do | What is current limitations |
|---|---|---|
| Documentation alignment | Make local docs agree on what is actually verified in this checkout. | The host gate and direct-entrypoint smoke harness are now rerun from this checkout, but manual DAW/runtime checks and any future firmware parity reruns still need their own dated evidence. |
| Repeatable checkout verification | Let a fresh checkout rerun the documented validation commands without manual environment repair. | `build_host.cmd` now handles both `Path`-only and duplicate-`PATH` shells, the unit-test payload emits under `build/unit_test_bin/<run-tag>/<config>/`, the latest wrapper-driven Release gate passed `232/232` in this checkout, and the documented DaisyHostCLI agent/CI adoption checks passed directly during the TF12 pass. |
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
| Richer live routing beyond the four topology presets | Expand the visible rack past simple entry/output and serial audio-only presets. | The visible two-node rack now exists, so the next value is additional route shapes, safer routing UX, or future cross-node signal surfaces without regressing the single-node path. |
| Deeper routing and node automation surfaces | Extend the current selected-node targeting and serial audio proof into richer routing, node-targeted events, and future rack automation surfaces. | The repo now proves live and offline two-node routing, so the next step is broadening what can flow between nodes without breaking the legacy single-node path. |
| Field firmware and hardware parity | Add real Field firmware parity, real hardware voltage output validation, hardware switches/LEDs, and Field-specific app ergonomics after the host-side control surface is stable. | The host now supports `daisy_field` shell metadata, K1-K8/CV1-CV4/Gate In/A1-B8 input mappings, host-side CV OUT/SW/LED evidence, startup-request launch planning, and selected-node Field surface render proof, so the remaining Field value is hardware-facing parity and deeper ergonomics rather than another board-selection seam. |

## Iteration Log

| Date | Confirmed working in this iteration | Problems observed in this iteration |
|---|---|---|
| 2026-04-27 | WS8 rack UX productionization is source-backed and verification-backed: red normalized-env Debug `unit_tests` first failed on missing rack topology/role display helpers; green Debug `unit_tests` built; targeted Debug rack/session/snapshot/board/render coverage passed `96/96`; focused Field/RSP status coverage passed `69/69`; Release render smoke passed for all current render scenarios; and full `cmd /c build_host.cmd` passed with Release `ctest` `216/216`, including standalone, render, and CLI smoke tests. Manager explanation: the existing two-node rack was polished with clearer topology direction, selected-node role labels, per-node context, and Patch/Field selected-node target hints without adding routing presets, mixed-board semantics, or new route behavior. | An earlier full-gate attempt hit `LNK1104` because `DaisyHost Hub.exe` was running, and one wrapper retry was aborted before returning output; the final wrapper rerun completed green. Manual visible GUI inspection, Field hardware validation, generated-adapter flashing, real CV voltage measurement, Field DAW/VST3 validation, DaisyHostController USB MIDI validation, mixed-board racks, and arbitrary firmware import were not performed. |
| 2026-04-26 | DaisyHost Field drawer/page foundation is source-backed and verification-backed: red Debug `unit_tests` first failed on missing `DaisyFieldKnobLayoutMode`, `DaisyFieldDrawerPage`, public-parameter-list, and drawer-page helpers; green Debug `cmake --build build --config Debug --target unit_tests` passed; Debug targeted `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest\|BoardProfileTest\|RenderRuntimeTest)"` passed `58/58`; Release targeted `ctest --test-dir build -C Release --output-on-failure -R "(BoardControlMappingTest\|BoardProfileTest\|RenderRuntimeTest)"` passed `58/58`; normalized-env Debug `cmake --build build --config Debug --target DaisyHostPatch_Standalone` passed and compiled the edited JUCE editor/processor sources; full `cmd /c build_host.cmd` passed with Release `ctest` `180/180`, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`. Manager explanation: Field now has circular Page 1/2/3 drawer navigation, Page 2 public parameter inventory, active alternative K mapping for remaining public controls, blank unused K slots, SW1/X previous and SW2/C next, and preserved default K1-K4 plus K5-K8 mapping behavior. | The first full-gate attempt in this iteration hit `LNK1104` on `build\DaisyHostHub_artefacts\Release\DaisyHost Hub.exe` while `DaisyHost Hub` and `DaisyHost Patch` were running, but the rerun completed green. The Debug standalone build emitted existing C4702 unreachable-code warnings in `DaisyHostPluginEditor.cpp`; no hardware/firmware work was touched. |
| 2026-04-25 | Adapter pipeline v0 is source-backed and verification-backed: red pytest first failed because `tools/generate_field_adapter.py` and `tools/audit_firmware_portability.py` did not exist; green pytest passed `3/3`; `field/MultiDelayGenerated` was generated from `tools/adapter_specs/field_multidelay.json`; generated firmware `make` passed with FLASH `121376 B` / `92.60%`; QAE passed `0 error(s), 0 warning(s)`; audit classified both `field/MultiDelay` and `field/MultiDelayGenerated` as `portable-core-ready`; targeted Debug CTest passed `52/52`; `cmd /c build_host.cmd` passed with Release `ctest` `159/159`. Manager explanation: DaisyHost now has a repeatable semi-automatic path from shared app core to Field firmware adapter, while arbitrary firmware translation remains out of scope. | Generated adapter flashing was not run because `make program` was optional in the plan and flashing is a hardware side effect. `field/MultiDelayGenerated` is build/QAE-verified only. Full manual hardware validation, arbitrary firmware import, mixed-board racks, routing changes, and DAW/VST3 validation remain deferred. |
| 2026-04-25 | PolyOsc is source-backed and verification-backed as a host-side DaisyHost app: red `cmake --build build --config Debug --target unit_tests` first failed on missing `DaisyPolyOscCore.h` and `apps/PolyOscCore.h`; green focused payload passed `11/11` for `AppRegistryTest.*:DaisyPolyOscCoreTest.*:PolyOscCoreTest.*:BoardControlMappingTest.*PolyOsc*:RenderRuntimeTest.*PolyOsc*`; raw Release `DaisyHostRender` first hit the known duplicate `Path` / `PATH` MSBuild issue, sanitized Release `DaisyHostRender` build passed; render smoke passed including `PolyOsc` and Daisy Field PolyOsc surface scenarios; `cmd /c build_host.cmd` passed with Release `ctest` `168/168`. Manager explanation: DaisyHost can now host the original Patch PolyOsc behavior with Patch controls and host-side Field K5 waveform mapping without adding Field firmware. | No firmware `make` was required because this pass stayed host-side. Field support is DaisyHost board-surface support only; real Field firmware, hardware voltage validation, mixed-board racks, and DAW/VST3 manual validation remain deferred. |
| 2026-04-25 | Sprint F3 added the first Daisy Field firmware adapter: `field/MultiDelay` builds with `make`, reuses shared `MultiDelayCore`, passes QAE validation, flashes through the connected ST-Link with OpenOCD `** Verified OK **`, and the DaisyHost regression gate still passes `159/159`. Manager explanation: this moves Field from host-only simulation toward hardware parity with the smallest useful adapter while keeping the host rack frozen. | The adapter is build-verified and flash-verified, but full hardware validation is not claimed yet. The remaining manual checklist is audio input 1 to outputs 1/2 with delay, K1-K5 behavior, K6-K8 no unexpected DSP effect, CV1 tertiary-delay input, CV OUT 1 `0..5V`, CV OUT 2 `0V`, SW1 impulse, SW2 OLED page toggle, LEDs/OLED stability, and no audio dropouts. |
| 2026-04-25 | Field ergonomics polish and board-generic UI cleanup are source-backed and verification-backed from this checkout: hosted apps now expose explicit Patch-page knob parameter ids where available, Field K5-K8 ranked extras exclude those K1-K4 targets, Field surface profile controls expose target ids for generic editor lookup, and the full wrapper-driven host gate passed `159/159`. | Raw Release target builds still need the wrapper or sanitized `Path` / `PATH` environment in this shell. Manual DAW/VST3 validation, Field firmware beyond the first `field/MultiDelay` adapter, full manual Field hardware validation, mixed-board racks, routing changes, and DAW automation expansion remain explicitly out of scope. |
| 2026-04-25 | Daisy Field native controls are source-backed and verification-backed from this checkout: `daisy_field` renders interactive K1-K8 and A1-B8 controls, maps K1-K4 to current Patch page bindings, maps K5-K8 to ranked automatable parameters, reuses host CV1-CV4/Gate In paths, supports `surface_control_set` render events, and the full wrapper-driven host gate passed `144/144` for that checkpoint. | This native-controls package was superseded later the same day by host-side Field outputs/switches/LEDs and selected-node refinement evidence. External DAW/VST3 validation, Field firmware, real hardware voltage output, Field-specific app ergonomics, and mixed-board racks remain explicitly out of scope. |
| 2026-04-24 | Daisy Field board-support shell is now source-backed and verification-backed from this checkout: `daisy_field` is registered through the board factory seam, accepted by Hub/session/standalone/render paths, rendered as passive board metadata, covered by a checked-in Field shell render smoke scenario, and the full wrapper-driven host gate passed `136/136`. | No code blocker after implementation; external DAW/VST3 validation remains deferred and Field native controls plus mixed-board racks were explicitly out of scope for that shell package. |
| 2026-04-24 | Full Release host verification is green again from this checkout: `cmd /c build_host.cmd` passed and `ctest` passed `128/128`, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`. Daisy Field readiness is now satisfied, and `WORKSTREAM_TRACKER.md` now carries the dedicated post-WS7 portfolio mirrored into `PROJECT_TRACKER.md`. | No new blocker in this iteration; external DAW/VST3 validation remains intentionally deferred until the later manual pass. |
| 2026-04-24 | `harmoniqs` and `vasynth` are now source-backed from this checkout: DaisyHost ships a portable `DaisyHarmoniqsCore` additive app and a portable `DaisyVASynthCore` seven-voice subtractive app, both registered as first-class hosted apps with checked-in render smoke scenarios. The targeted Debug payload run passed `18/18`, and Release render smoke now passes for MultiDelay, Torus, CloudSeed, Braids, Harmoniqs, and VA Synth scenarios. | Full Release `ctest` is still blocked by the existing machine-level read/lock issue on the freshly linked Release payload, and direct `ctest -C Debug -R ...` can still point at a stale run-tag path in this checkout, so the latest proof remains targeted-Debug-payload-backed plus Release-render-smoke-backed rather than a fully green Release host gate. |
| 2026-04-24 | Braids verification was stronger than the paused state: the Debug unit-test runner passed `110/110`, the focused Braids/render-runtime subset passed `27/27`, and the Release smoke harness passed for standalone plus MultiDelay, Torus, CloudSeed, and Braids render scenarios. `build_host.ps1` handled `Path`-only elevated shells, and the unit-test runner was emitted under `build/test_bin/<config>/`. | Full Release `ctest` could not launch the Release unit-test runner in that pass (`resource busy or locked`), so that proof was Debug-unit-backed plus Release-smoke-backed rather than a fully green Release host gate. |
| 2026-04-23 | `braids` is now source-backed, build-backed, and smoke-backed from this checkout: DaisyHost ships a portable `DaisyBraidsCore`, a percussion-first hosted app with `Drum` / `Finish` knob pages, MIDI-first triggering, gate alias support, and a checked-in `braids_smoke.json` render scenario. | A fresh full host gate was not rerun against the final visible-rack plus `braids` code because `DaisyHostUnitTestsRunner.exe` can still be held open or disappear after link in this shell; external DAW/VST3 validation remains intentionally deferred until a later post-WS7 manual pass. |
| 2026-04-23 | Visible WS7 rack sprint is now source-backed, build-backed, and smoke-backed from this checkout: the plugin/standalone now run a visible two-node live rack with selected-node targeting, four audio-only topology presets, a board factory seam, `HostSessionState` v5 rack globals, and direct forward/reverse two-node render proofs. | A fresh full host gate was not rerun against the final visible-rack code because `DaisyHostUnitTestsRunner.exe` is currently held open by another process in this shell; external DAW/VST3 validation remains intentionally deferred until a later post-WS7 manual pass. |
| 2026-04-23 | WS6 landed and WS7 groundwork landed from this checkout: `multidelay` and `cloudseed` now expose named MetaControllers, `cloudseed` is treated as a first-class supported hosted app, `HostSessionState` now round-trips v4 node/route records, render scenarios/manifests now accept optional `nodes[]` / `routes[]`, the hidden two-node audio-chain proof passes, and the full wrapper-driven host gate reran green at `89/89`. | External DAW/VST3 load validation remains intentionally deferred until a later post-WS7 manual pass; the build still inherits the shell-local `Path` / `PATH` split if you bypass `build_host.cmd`. |
| 2026-04-22 | Workstream-6 pilot landed from this checkout: DaisyHost now hosts `cloudseed` on top of a new portable `DaisyCloudSeedCore`, page-based `Space` / `Motion` knob remapping is live in the hosted app and the JUCE processor binding refresh, `cloudseed_smoke.json` is checked in, and the full host gate reran green at `79/79`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must still be run in a sanitized one-path shell; upstream `CloudSeedCore` also needed a local MSVC portability fix because `RandomBuffer.cpp` used a VLA. |
| 2026-04-22 | `build_host.cmd` now reruns the full DaisyHost host gate from this checkout, `build_host.ps1` encapsulates the local `Path` / `PATH` normalization, the VST3 manifest step is now PowerShell-backed and rerunnable on Windows, and fresh firmware reference builds passed in `patch/MultiDelay/` and `patch/Torus/`. | External DAW/VST3 load validation is intentionally deferred until post-WS7; `patch/Torus` still needs a PowerShell `build/` removal for a clean rebuild because `make clean` calls `rm` on Windows. |
| 2026-04-22 | Workstreams 4 and 5 landed from this checkout: DaisyHost now builds a fixed five-slot DAW automation bridge plus an in-process effective-state snapshot API, `unit_tests` gained bridge/readback coverage, and the full host gate reran successfully with `71/71` tests green. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell; external DAW/VST3 load validation is still manual. |
| 2026-04-22 | `tests/run_smoke.py` landed and the full DaisyHost host gate reran successfully from this checkout: `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone` rebuilt in `Release`, then `ctest` passed `65/65`, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell. |
| 2026-04-22 | Full DaisyHost host gate reran successfully from this checkout: `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone` rebuilt in `Release`, then `ctest` passed `63/63`. | This Codex PowerShell shell still exports both `Path` and `PATH`, so MSBuild commands must be run in a sanitized one-path shell. |
| 2026-04-22 | Runtime verification partially restored from this checkout: `cmake -S . -B build` succeeded, `unit_tests` built and `ctest` passed `63/63`, and `DaisyHostHub`, `DaisyHostRender`, and `DaisyHostPatch_VST3` rebuilt successfully. | The default shell exports both `Path` and `PATH`, which breaks MSBuild until normalized, and the running `build\\DaisyHostPatch_artefacts\\Release\\Standalone\\DaisyHost Patch.exe` blocked the standalone relink with `LNK1104`. |
| 2026-04-22 | Documentation/governance hardening completed: `PROJECT_TRACKER.md` promoted to first-class tracking doc; `README.md`, `AGENTS.md`, `CHECKPOINT.md`, `CHANGELOG.md`, and `LATEST_PROJECTS.md` aligned around tracker usage and mandatory per-iteration testing; `ROADMAP.d2` and `ROADMAP.svg` added. | `cmake` and `make` remain unavailable on `PATH`, and `DaisyHost/build` is absent, so runtime build/test claims remain historical until rerun. |
| 2026-04-22 | `ROADMAP.d2` was restructured into stacked governance, workstream, iteration, and ownership bands and recompiled to a more preview-friendly `ROADMAP.svg`. | Runtime build/test tooling is still outside the scope of this docs-only pass, so no new host or firmware verification was attempted. |
| 2026-04-22 | `SKILL_PLAYBOOK.md` was added as the dedicated DaisyHost file for skill-related activities, and the tracker now distinguishes `Expected UF` from evidence-based `Observed UF`. | Most skills still have no DaisyHost-local observed sample set, so `Observed UF` remains pending until future task iterations log real evidence. |
