# DaisyHost Checkpoint

## Snapshot

- Date: 2026-04-26
- Workspace: `DaisyHost/`
- Current CMake version in source: `0.2.0`
- Active refresh target: `0.2.0`
- Scope: host-side Daisy Patch plugin and standalone app with a multi-app host
  layer, `MultiDelay` as the default regression fixture, `Torus` as the first
  second app, first-class `CloudSeed`, `Braids`, `Harmoniqs`, `VA Synth`,
  `PolyOsc`, and `Subharmoniq` support, named MetaControllers for
  `multidelay` and `cloudseed`, plus a
  visible two-node live rack, board factory seam, Field board-support shell,
  host-side Field native controls, Field extended host surface support,
  forward/reverse two-node audio-chain proof paths, and the first
  flash-verified Daisy Field firmware adapter under `field/MultiDelay`, plus
  adapter-pipeline v0 tooling that generates a build/QAE-verified
  `field/MultiDelayGenerated` adapter from a JSON spec, and a
  build/QAE/ST-Link flash-verified `field/SubharmoniqField` firmware target
  backed by the portable `DaisySubharmoniqCore`, plus a build/QAE/ST-Link flash-verified
  `field/DaisyHostController` USB MIDI controller firmware target

This checkpoint records the current source-backed state plus the last recorded
runtime verification. `PROJECT_TRACKER.md` is now the active ordered roadmap
and per-iteration testing ledger for DaisyHost.

## Companion Tracking Files

- `PROJECT_TRACKER.md`: current ordered roadmap, per-iteration testing ledger,
  and concurrent-thread coordination
- `SKILL_PLAYBOOK.md`: dedicated skill-selection, `Expected UF` /
  `Observed UF`, evidence, and validation log
- `CHANGELOG.md`: canonical durable release and workflow policy history
- `README.md`: workspace overview and local-doc entrypoint

## DaisyBrain NotebookLM Notebook

Curated repo knowledge notebook created for Daisy and DaisyExamples
architecture questions, planning, and design recall:

- Notebook title: `DaisyBrain`
- Notebook id: `8084395c-2d50-464c-967b-7569926fe771`
- Preferred local CLI home: `C:\Users\denko\.notebooklm-daisybrain`

Current seeded sources:

- `docs/notebooklm/DaisyBrain-knowledge-pack.txt`
- repo `AGENTS.md`
- `LATEST_PROJECTS.md`
- `DaisyHost/README.md`
- `DaisyHost/CHECKPOINT.md`
- `DaisyHost/CHANGELOG.md`
- `docs/plans/2026-04-18-daisyhost-book-roadmap.md`
- `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

CLI-first usage notes:

```sh
$env:NOTEBOOKLM_HOME = "$HOME\.notebooklm-daisybrain"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" ask -n 8084395c-2d50-464c-967b-7569926fe771 "question"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" source list -n 8084395c-2d50-464c-967b-7569926fe771 --json
```

Operational note:

- Prefer the NotebookLM CLI over MCP for this notebook when local automation is
  needed.
- Use the notebook for architectural recall, DaisyHost roadmap questions,
  Daisy-vs-host tradeoffs, and future book-feature planning.

## Current Build Commands

Preferred local entrypoint from `DaisyHost/`:

```sh
.\build_host.cmd
```

That wrapper launches `build_host.ps1`, normalizes the `Path` / `PATH`
environment split for MSBuild-backed commands in this shell, injects a fresh
`DAISYHOST_UNIT_TEST_RUN_TAG` for the Release unit-test payload path, and
reruns the full host gate.

Underlying raw commands:

```sh
cmake -S . -B build
cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir build -C Release --output-on-failure
```

`ctest` now includes:

- `unit_tests`
- `DaisyHostStandaloneSmoke`
- `DaisyHostRenderSmoke`
- `DaisyHostCliListApps`
- `DaisyHostCliDescribeApp`
- `DaisyHostCliDescribeBoard`
- `DaisyHostCliValidateScenario`
- `DaisyHostCliRender`

Current shell note:

- the rebuilt `unit_tests` target is now emitted as a Release payload under
  `build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
- `ctest` launches unit cases through `tests/run_unit_test_payload.py`, which
  receives that payload path and attempts to run a fresh temporary copy
- the wrapper-driven full host gate reran green on 2026-04-26, so the current
  baseline in this checkout is again a fully green Release host gate rather
  than only partial smoke/debug proof
- after local Debug rebuilds, direct `ctest -C Debug -R ...` can still point at
  a stale unbuilt run-tag path in this checkout; when that happens, the current
  safe targeted fallback is direct execution through
  `py -3 tests/run_unit_test_payload.py <fresh-payload> --gtest_filter=...`
- `tests/run_smoke.py` now uses a wider process-query timeout for standalone
  smoke so slower Windows process-path discovery does not produce a false
  timeout on an otherwise healthy launch
- result: the rack freeze gate is no longer blocked in this checkout, the
  Daisy Field board-support shell, host-side Field native controls, Field
  extended host surface support, host-side `polyosc` app import, and
  `subharmoniq` hosted app are implemented, native `DaisyHostCLI.exe` is
  build/gate-verified for agent and CI workflows, the latest host gate passed
  `202/202`, the first Field firmware adapter (`field/MultiDelay`) is
  build-verified and ST-Link flash-verified, and `field/SubharmoniqField` is
  build/QAE/ST-Link flash-verified with host-tested playable defaults; both
  Field hardware manual checklists remain pending

Rebuild the Patch firmware reference targets only when DaisyHost shared cores or
firmware adapters change:

```sh
make
```

from `patch/MultiDelay/`.

and:

```sh
make
```

from `patch/Torus/`.

## Last Recorded Runtime Verification

- Last fully green DaisyHost host build/test verification rerun from this
  checkout: 2026-04-26
- Verified commands/results in the current 2026-04-26 TF12 verification/build
  hardening pass:
  - manager-readable result: TF12 is a documentation and verification adoption
    slice, not a new CLI command or product-behavior pass. The current truth is
    the fresh `202/202` full host gate, and older `196/196` and `197/197`
    results remain dated historical evidence only.
  - preflight:
    - `git status --short`: reviewed; checkout is broadly dirty across host,
      docs, firmware, submodules, build outputs, and untracked files
    - `git diff --name-status --`: reviewed; TF12 did not revert, stage,
      delete, or normalize unrelated work
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `202/202`
  - direct DaisyHostCLI adoption checks:
    - `build\Release\DaisyHostCLI.exe doctor --build-dir build --source-dir . --config Release --json`:
      passed
    - `build\Release\DaisyHostCLI.exe list-apps --json`: passed
    - `build\Release\DaisyHostCLI.exe describe-app cloudseed --json`: passed
    - `build\Release\DaisyHostCLI.exe describe-board daisy_field --json`:
      passed
    - `build\Release\DaisyHostCLI.exe validate-scenario training\examples\multidelay_smoke.json --json`:
      passed
    - `build\Release\DaisyHostCLI.exe render training\examples\multidelay_smoke.json --output-dir build\cli_smoke\tf12_multidelay --json`:
      passed, audio checksum `c9c3f665e6a0dd2b`
    - `build\Release\DaisyHostCLI.exe smoke --mode render --build-dir build --source-dir . --config Release --json`:
      passed
  - caveats:
    - no new DaisyHostCLI commands were added
    - manual DAW/VST3 validation, standalone icon visibility, computer-side USB
      MIDI validation, and hands-on Field audio/control/CV validation were not
      run
- Verified commands/results in the current 2026-04-26 DaisyHostCLI pass:
  - manager-readable result: `DaisyHostCLI.exe` now exposes native agent/CI
    commands for app/board/input discovery, app/board JSON descriptions,
    scenario validation, offline render, effective-state snapshots, render
    smoke delegation, and build-artifact doctor checks.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests`: failed on
      missing `daisyhost/CliPayloads.h`
  - targeted host proof:
    - `.\build_host.cmd -Configuration Debug -SkipTests`: passed for
      `unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`
    - `ctest --test-dir build -C Debug --output-on-failure -R "CliPayloadsTest"`:
      passed, `4/4`
    - direct Debug CLI checks for `list-apps`, `describe-app cloudseed`,
      `describe-board daisy_field`, `validate-scenario`, `snapshot`,
      `doctor`, `render`, and `smoke --mode render`: passed
    - `ctest --test-dir build -C Debug --output-on-failure -R "DaisyHostCli"`:
      passed, `5/5`
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `197/197`
    - Release `DaisyHostCLI.exe smoke --mode render --build-dir build --source-dir . --config Release --json`:
      passed
    - Release `DaisyHostCLI.exe doctor --build-dir build --source-dir . --config Release --json`:
      passed
  - caveats:
    - this was host-side only; no firmware path was touched
    - raw MSBuild still needs the wrapper or one-Path environment normalization
- Verified commands/results in the current 2026-04-26 Field Page 1 rollback
  after the oversized modulation-bay redesign:
  - manager-readable result: the last Page 1 enlargement/keyboard-helper
    experiment is reverted. RSP Page 1 is back to the compact split layout:
    Keyboard MIDI, octave, visible keyboard, and MIDI tracker on the left;
    compact CV generator controls, live CV bars, and Gate 1/2 on the right.
    Earlier stable Field behavior remains: three RSP pages, SW1/X and SW2/C
    hosted-app navigation, `.1`/`.2` CV target labels, non-overlapping
    default/alternative K targets, unsafe target filtering, and CV generator
    stability.
  - targeted host proof:
    - `rg -n "VisibleKeyboardStartForOctave|kKeyboardMinMidiNote|kKeyboardMaxMidiNote|BaseMidiNoteForOctave" include src tests`:
      returned no matches
    - `git diff --check -- src\juce\DaisyHostPluginEditor.cpp include\daisyhost\ComputerKeyboardMidi.h src\ComputerKeyboardMidi.cpp tests\test_computer_keyboard_midi.cpp`:
      passed, with line-ending warnings only
    - `cmake --build build --config Debug --target unit_tests DaisyHostPatch_Standalone`:
      passed, with existing `DaisyHostPluginEditor.cpp` C4702 warnings
    - `ctest --test-dir build -C Debug --output-on-failure -R "(ComputerKeyboardMidiTest|BoardControlMappingTest|SignalGeneratorTest)"`:
      passed, `27/27`
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `196/196`
  - caveats:
    - this was host-side only; no firmware path was touched
    - automated visual screenshot capture was not run
    - the reverted expanded modulation bay, Page 3 status move, and
      `VisibleKeyboardStartForOctave` helper are not current behavior
- Verified commands/results in the current 2026-04-26 DaisyHostController
  Field USB MIDI controller firmware pass:
  - manager-readable result: `field/DaisyHostController` now builds and
    flashes as a controller-only Daisy Field firmware target that sends
    standard USB MIDI for DaisyHost control. The mapping is K1-K8 -> CC 20-27,
    CV1-CV4 -> CC 28-31, A1-B8 -> notes 60-75, and SW1/SW2 -> momentary
    CC 80/81 on MIDI channel 1.
  - red proof:
    - `py -3 -m pytest -q tests\test_daisyhost_controller_firmware.py`:
      failed because `field/DaisyHostController/Makefile` did not exist yet
  - targeted proof:
    - `py -3 -m pytest -q tests\test_daisyhost_controller_firmware.py`:
      passed, `1/1`
    - `make` from `field/DaisyHostController`: passed; FLASH `98252 B` /
      `74.96%`
    - `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .`:
      passed, `0 error(s), 0 warning(s)`
    - `make` from `field/DaisyHostController`: passed as up to date
    - `make program` from `field/DaisyHostController`: passed; OpenOCD
      detected STLINK V3, target voltage `3.238171`, programmed
      `build/DaisyHostController.elf`, reported `** Verified OK **`, and reset
      the target
    - user manual check: all knobs, buttons, and keys displayed correctly on
      the Field screen
  - caveats:
    - the first QAE command without `PYTHONIOENCODING=utf-8` failed before
      findings due to Windows cp1252 Unicode output encoding
    - USB MIDI enumeration, DaisyHost standalone MIDI learn, DAW/VST3 routing,
      and computer-side MIDI validation were not run
- Verified commands/results in the current 2026-04-26 Field CV target menu and
  generator visibility refinement:
  - manager-readable result: DaisyHost Field now keeps the normal and
    alternative K mappings distinct. The alternative layout excludes every
    default Field K target, and CV target dropdowns expose both safe sets as
    `Kx.1` for normal/default Field targets and `Kx.2` for alternative/public
    parameter targets. RSP Page 1 now exposes all four CV generators with
    mode, waveform, target, Offset, Amp Vp, and Freq controls while preserving
    the live momentary CV bars.
  - targeted host proof:
    - `cmake --build build --config Debug --target unit_tests DaisyHostPatch_Standalone`:
      passed, with existing `DaisyHostPluginEditor.cpp` C4702 warnings
    - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest|SignalGeneratorTest)"`:
      passed, `66/66`
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `187/187`
  - caveats:
    - this was host-side only; no firmware path was touched
    - manual standalone visual/audio QA was not run
- Verified commands/results in the current 2026-04-26 Field CV generator
  stability pass:
  - manager-readable result: DaisyHost Field CV generators no longer use one
    generated CV lane to drive both an app CV input port and a latched
    knob/parameter target. Latched targets own their lane, and audio-critical
    targets are hidden/skipped by default so a generator cannot sweep `Mix`,
    input/output, mute/bypass/enabled, level, or volume-style controls into
    disappearing-output states.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests`: failed because
      `ShouldForwardDaisyFieldCvInput` did not exist yet
  - targeted host proof:
    - `cmake --build build --config Debug --target unit_tests`: passed
    - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SignalGeneratorTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest)"`:
      passed, `64/64`
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `185/185`
  - caveats:
    - this was host-side only; no firmware path was touched
    - manual standalone audio QA with CloudSeed/MultiDelay CV generators still
      needs hands-on confirmation
- Verified commands/results in the current 2026-04-26 Field UI/control
  separation pass:
  - manager-readable result: DaisyHost Field now separates the hosted app menu
    from the right-side program drawer. SW1/X and SW2/C rotate the hosted app
    menu only; the RSP Page 1/2/3 drawer remains mouse/debug controlled.
    RSP Page 1 now has latched CV target dropdowns for knob-controlled
    parameters, and selecting a target switches that CV source to Generator.
    CloudSeed exposes Space, Motion, Arp, and Advanced app pages; Advanced maps
    the Field knobs to `eq_low_freq`, `eq_high_freq`, `eq_cutoff`,
    `eq_low_gain`, `eq_high_gain`, `eq_cross_seed`, `seed_diffusion`, and
    `seed_delay`.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests`: failed for
      missing `BuildDaisyFieldCvTargetOptions`, missing CloudSeed page fields,
      and missing Field knob override bindings
  - targeted host proof:
    - `cmake --build build --config Debug --target unit_tests`: passed
    - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|CloudSeedCoreTest|DaisyCloudSeedCoreTest|RenderRuntimeTest)"`:
      passed, `59/59`
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `183/183`
  - caveats:
    - MP4 inspection was unavailable because `ffmpeg`, `cv2`, `imageio`, and
      `moviepy` were not installed
    - no firmware or hardware paths were touched
- Verified commands/results in the current 2026-04-26 CloudSeed arpeggiator
  pass:
  - manager-readable result: `cloudseed` now has a deterministic
    parameter-based arpeggiator that rhythmically steps selected CloudSeed
    performance parameters through effective-state modulation. It is not a
    MIDI-note arpeggiator; CloudSeed remains an audio-input reverb/effect.
  - red proof:
    - direct Debug payload for
      `DaisyCloudSeedCoreTest.*:CloudSeedCoreTest.*:RenderRuntimeTest.ProducesDeterministicCloudSeedRenderWithMenuActions`
      first failed for missing `arp_*` canonical parameters, missing
      `node0/menu/arp/*` menu ids, and render-runtime rejection of
      `node0/menu/arp/enabled`
  - targeted green proof:
    - `cmake --build build --config Debug --target unit_tests`: passed
    - direct Debug payload for
      `DaisyCloudSeedCoreTest.*:CloudSeedCoreTest.*:RenderRuntimeTest.ProducesDeterministicCloudSeedRenderWithMenuActions:RenderRuntimeTest.PolyOscFieldSurfaceMapsK5ToWaveformOnly:RenderRuntimeTest.FieldExtendedSurfaceStateMirrorsOutputsSwitchesAndLeds`:
      passed, `13/13`
    - sanitized `cmake --build build --config Release --target unit_tests`:
      passed
    - direct Release payload for the same `13/13` filter: passed
    - sanitized `cmake --build build --config Release --target DaisyHostRender`:
      passed
    - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`:
      passed
  - full host-gate caveat:
    - `cmd /c build_host.cmd` configured and built `unit_tests`, but failed
      before `ctest` because `build\DaisyHostHub_artefacts\Release\DaisyHost Hub.exe`
      was locked by a running `DaisyHost Hub` process (`PID 78928`), producing
      `LNK1104`
- Verified commands/results in the current 2026-04-26 Subharmoniq hosted-app,
  no-audio fix, Field UI refinement, and Field firmware pass:
  - manager-readable result: DaisyHost now has a first-class `subharmoniq`
    hosted app plus a real `field/SubharmoniqField` firmware target. Round 1
    implements a Subharmonicon-inspired portable core with six oscillator
    sources, two four-step sequencers, four integer rhythm dividers, quantize
    modes, SVF-style low-pass filtering, VCF/VCA envelopes, CV/Gate/MIDI
    mapping, OLED/menu state, Field K1-K8/A-B/SW1-SW2 controls, and an
    internal tempo clock so `B7` play can produce rhythmic envelope triggers
    without external MIDI clock or Gate In. The follow-up audible-default fix
    changes the envelope from a one-sample attack blip to a real attack/decay
    phase, raises the default filter/output/source levels, and makes Field
    K1-K8 startup pickup-style so physical knob positions do not immediately
    overwrite the safe patch.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests` first failed
      because `src/DaisySubharmoniqCore.cpp` did not exist
    - `make` from `field/SubharmoniqField` first failed on const
      `MidiEvent::AsNoteOn()` adapter use before the firmware fix
    - `ctest --test-dir build -C Debug --output-on-failure -R "SubharmoniqCoreTest.PlayToggleRunsInternalClockAndProducesAudio"`:
      failed before the no-audio fix with `GetTriggerCount() == 0` and peak
      audio `0`
    - after the internal-clock fix, the same test was tightened for usable
      level and failed before the audible-default fix with peak `0.00323891826`
      versus required `0.02`, and energy `0.383889765` versus required `1.0`
  - targeted host proof:
    - `cmake --build build --config Debug --target unit_tests`: passed after
      the audible-default fix
    - `ctest --test-dir build -C Debug --output-on-failure -R SubharmoniqCoreTest`:
      passed, `6/6`
    - `ctest --test-dir build -C Debug --output-on-failure -R "(SubharmoniqCoreTest|AppRegistryTest|BoardControlMappingTest)"`:
      passed, `20/20`
    - after the SW1/SW2 pseudo-encoder mapping change, the first wrapper gate
      failed only on old Field switch expectations; targeted rerun passed
      `RenderRuntimeTest.PolyOscFieldSurfaceMapsK5ToWaveformOnly`,
      `RenderRuntimeTest.FieldExtendedSurfaceStateMirrorsOutputsSwitchesAndLeds`,
      and `DaisyHostRenderSmoke`
  - firmware proof:
    - `cd ..\field\SubharmoniqField; make`: passed; FLASH `124520 B` /
      `95.00%`
    - `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .`:
      passed, `0 error(s), 0 warning(s)`
    - follow-up `cd ..\field\SubharmoniqField; make`: passed as up to date
    - follow-up `cd ..\field\SubharmoniqField; make program`: passed; OpenOCD
      detected STLINK `V3J7M2`, target voltage `3.263618`, programmed
      `build/SubharmoniqField.elf`, reported `** Verified OK **`, and reset
      the target
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `175/175`
  - caveats:
    - manual Field audio/control/CV/MIDI/OLED/LED validation was not performed
    - Round 2 filter switching between SVF LPF/BPF and ladder-style LPF remains
      intentionally deferred
- Verified commands/results in the current 2026-04-25 PolyOsc hosted-app
  import:
  - manager-readable result: DaisyHost now hosts the original Patch `PolyOsc`
    behavior as `polyosc` without adding new firmware. Patch K1-K3 control the
    three oscillator frequencies, K4 applies the global frequency offset, the
    encoder selects waveform, outputs 1-3 carry individual oscillators, and
    output 4 is the mixed host/render output. Field support is host-side only:
    K1-K4 mirror Patch controls and K5 maps to `waveform`.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests`: failed first
      because `daisyhost/DaisyPolyOscCore.h` and
      `daisyhost/apps/PolyOscCore.h` did not exist
  - green targeted proof:
    - `cmake --build build --config Debug --target unit_tests`: passed
    - `py -3 tests\run_unit_test_payload.py build\unit_test_bin\20260425214512302\Debug\DaisyHostTestPayload.bin --gtest_filter="AppRegistryTest.*:DaisyPolyOscCoreTest.*:PolyOscCoreTest.*:BoardControlMappingTest.*PolyOsc*:RenderRuntimeTest.*PolyOsc*"`:
      passed, `11/11`
  - render proof:
    - raw `cmake --build build --config Release --target DaisyHostRender`
      first hit the known duplicate `Path` / `PATH` MSBuild environment issue
    - sanitized `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostRender`:
      passed
    - `py -3 tests\run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`:
      passed, including `PolyOsc` and Daisy Field PolyOsc surface scenarios
  - full host gate:
    - `cmd /c build_host.cmd`: passed
    - underlying host-gate steps executed by the wrapper:
      - `cmake -S . -B build`: passed
      - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
        passed
      - `ctest --test-dir build -C Release --output-on-failure`: passed,
        `168/168`
  - caveats:
    - no firmware `make` was required because this pass stayed host-side
    - real Field firmware, hardware voltage validation, mixed-board racks, and
      DAW/VST3 manual validation remain deferred
- Verified commands/results in the current 2026-04-25 HW/App adapter pipeline
  v0 pass:
  - manager-readable result: DaisyHost now has a semi-automatic shared-core to
    Field-firmware adapter pipeline. This is intentionally not a full
    libDaisy/STM32 firmware translator; it generates adapter glue around a
    portable DaisyHost app core and audits existing firmware for portability.
  - implementation scope:
    - `tools/generate_field_adapter.py`
    - `tools/adapter_specs/field_multidelay.json`
    - `tools/audit_firmware_portability.py`
    - `tests/test_field_adapter_generator.py`
    - `field/MultiDelayGenerated/*`
  - red proof:
    - `py -3 -m pytest -q tests/test_field_adapter_generator.py -p no:cacheprovider`
      first failed because the generator and audit scripts did not exist
  - `py -3 -m pytest -q tests/test_field_adapter_generator.py -p no:cacheprovider`:
    passed, `3/3`
  - `py -3 tools/generate_field_adapter.py --spec tools/adapter_specs/field_multidelay.json --out ..\field\MultiDelayGenerated`:
    passed
  - `cd ..\field\MultiDelayGenerated; make`: passed; FLASH `121376 B` /
    `92.60%`
  - `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .`:
    passed, `0 error(s), 0 warning(s)`
  - `py -3 tools/audit_firmware_portability.py ..\field\MultiDelay --json`:
    passed and classified the hand-written adapter as `portable-core-ready`
  - `py -3 tools/audit_firmware_portability.py ..\field\MultiDelayGenerated --json`:
    passed and classified the generated adapter as `portable-core-ready`
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(MultiDelayCoreTest|BoardControlMappingTest|RenderRuntimeTest)"`:
    passed, `52/52`
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
      passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed,
      `159/159`
  - caveats:
    - `make program` was not run for `field/MultiDelayGenerated`; generated
      adapter status is build/QAE-verified only
    - arbitrary firmware import remains out of scope; the audit tool reports
      required extraction work instead of rewriting firmware
- Verified commands/results in the current 2026-04-25 Sprint F3 Field
  hardware/firmware parity adapter pass:
  - manager-readable result: DaisyHost now has its first hardware-facing Daisy
    Field firmware adapter, `field/MultiDelay`. This uses the same shared
    `MultiDelayCore` regression fixture as the host and Patch firmware adapter,
    so Field hardware proof can begin without changing the DaisyHost rack,
    session, render, or DAW behavior.
  - implementation scope:
    - `field/MultiDelay/Makefile`
    - `field/MultiDelay/MultiDelay.cpp`
    - `field/MultiDelay/README.md`
    - `field/MultiDelay/CONTROLS.md`
  - red proof:
    - `cd ..\field\MultiDelay; make` first failed as expected because
      `MultiDelay.cpp` did not exist yet:
      `No rule to make target 'build/MultiDelay.o'`
  - firmware build notes:
    - first full link with default optimization overflowed Field flash by
      `7800 bytes`
    - adding `OPT = -Os` in `field/MultiDelay/Makefile` reduced firmware size
      enough for Daisy Field flash
  - `cd ..\field\MultiDelay; make`: passed
  - `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .`:
    passed, `0 error(s), 0 warning(s)`
  - `cd ..\field\MultiDelay; make program`: passed; OpenOCD detected STLINK
    `V3J7M2`, target voltage `3.271571`, programmed the STM32H7 target, and
    reported `** Verified OK **`
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(MultiDelayCoreTest|BoardControlMappingTest|RenderRuntimeTest)"`:
    passed, `52/52`
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
      passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed,
      `159/159`
  - caveats:
    - ST-Link flash/verify is proven, but full manual Field hardware
      validation is not yet claimed
    - remaining manual checklist: audio input 1 to outputs 1/2 with delay,
      K1-K5 behavior, K6-K8 no unexpected DSP effect, CV1 tertiary-delay
      input, CV OUT 1 `0..5V`, CV OUT 2 `0V`, SW1 impulse, SW2 OLED status
      page toggle, LEDs/OLED stability, and no audio dropouts
    - no DaisyHost host runtime change, mixed-board rack behavior, routing
      change, DAW/VST3 validation, or DAW automation expansion belongs to this
      pass
- Verified commands/results in the current 2026-04-25 Field ergonomics polish
  and board-generic UI cleanup full-gate pass:
  - manager-readable result: DaisyHost Field now behaves more intentionally
    without opening new architecture. Field K5-K8 ranked extras avoid
    duplicating the real K1-K4 parameter targets where hosted apps expose that
    metadata, and the Field editor uses board-profile target metadata for
    knobs, keys, and switches instead of Field-only string construction. This
    improves operator value and future board-support maintainability while
    keeping `daisy_patch` as the default and the rack topology frozen.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests` first failed on
      missing `HostedAppPatchBindings::knobParameterIds`
    - `ctest --test-dir build -C Debug --output-on-failure -R "BoardProfileTest"`
      failed `10/11` because Field surface controls did not yet expose target
      ids usable by generic editor lookup
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|RenderRuntimeTest|HostSessionStateTest|LiveRackTopologyTest)"`: passed, `70/70`
  - raw `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`:
    first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Release target build:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`:
    passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including Field native controls, Field extended surface, and selected-node Field surface scenarios
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `159/159`
  - caveats:
    - manual DAW/VST3 validation remains deferred
    - this pass does not add Field firmware, real hardware voltage output,
      mixed-board racks, routing changes, or DAW automation expansion
- Verified commands/results in the current 2026-04-25 knob-drag crash fix
  full-gate pass:
  - manager-readable result: DaisyHost fixed a board/app-independent crash
    reported while moving knobs. The root cause was shared hosted-app core
    state being accessed from both the audio thread and message-thread
    control/UI refresh paths during live knob drags. The fix serializes hosted
    core access and selected-node/core pointer handoff, so the same protection
    applies to all hosted apps on both `daisy_patch` and `daisy_field`.
  - implementation scope:
    `src/juce/DaisyHostPluginProcessor.h` and
    `src/juce/DaisyHostPluginProcessor.cpp`
  - `cmake --build build --config Debug --target unit_tests`: passed
  - raw `cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Debug standalone build rerun:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `66/66`
  - visible Debug standalone validation:
    - launched `DaisyHost Patch.exe --board daisy_field --app multidelay`,
      dragged Field knobs, and confirmed the process remained alive/responding
    - launched `DaisyHost Patch.exe --board daisy_patch --app multidelay`,
      dragged Patch knobs, and confirmed the process remained alive/responding
  - sanitized Release target build:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
    passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `155/155`
  - caveats:
    - no automated red regression test was added because the crash is a JUCE
      audio/message-thread race outside the current unit-test harness
    - DAW/VST3 manual knob-drag validation remains deferred
    - this fix does not change `daisy_patch` default behavior, rack topology,
      Field firmware status, or real hardware validation status
- Verified commands/results in the current 2026-04-25 Field refinement closeout
  full-gate pass:
  - manager-readable result: DaisyHost now proves Field selection and selected
    node behavior more clearly. Hub `Play / Test` launch planning writes the
    supported startup request, Field render/smoke includes selected-node
    surface evidence for `node1`, and the Field UI copy no longer describes
    Field controls as Patch controls.
  - red proof:
    `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"`
    first failed because Hub play plans still used command-line args only and
    Field surface manifests still reported `node0` for a selected `node1`
    scenario
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"`: passed, `39/39`
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|BoardControlMappingTest|HubSupportTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `76/76`
  - raw `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_Standalone`: first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Release build rerun:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_Standalone`: passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including the new `training/examples/field_node_target_surface_smoke.json`
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `155/155`
  - this rerun verifies host-side Field selection/startup-request planning and
    selected-node Field surface evidence; it does not verify Field firmware,
    real hardware voltage output, mixed-board racks, or manual DAW/VST3 loading
- Verified commands/results in the current 2026-04-25 Field extended host
  surface full-gate pass:
  - manager-readable result: DaisyHost now automatically tests `daisy_field`
    as a functioning host-side Field board surface. This closes the desktop
    model gap between "Field can be selected and played" and "Field exposes
    repeatable host-side outputs/switches/LEDs" while keeping the rack frozen
    and `daisy_patch` as the default.
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|EffectiveHostStateSnapshotTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `68/68`
  - `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`: passed after normalizing the shell-local duplicate `Path` / `PATH` environment through a Python subprocess wrapper
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including the Field shell, Field native controls, and Field extended surface scenarios
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `152/152`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the render smoke harness now covers the Field board-support shell scenario,
    the Field native controls scenario, and
    `training/examples/field_extended_surface_smoke.json`
  - this rerun verifies host-side Field outputs/switches/LEDs from this
    checkout; it does not verify Field firmware, real hardware voltage output,
    mixed-board racks, or manual DAW/VST3 loading
- Verified commands/results in the earlier 2026-04-25 full-gate pass:
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `144/144`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the render smoke harness now covers the Field board-support shell scenario
    and the Field native controls scenario
  - this rerun verifies host-side Field native controls from this checkout
- Verified commands/results in the earlier 2026-04-24 full-gate pass:
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `136/136`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the aggregate now includes the first-class hosted-app wave through:
    - `multidelay`
    - `torus`
    - `cloudseed`
    - `braids`
    - `harmoniqs`
    - `vasynth`
    - `polyosc`
  - this rerun verified the Daisy Field board-support shell from this checkout
- Verified commands/results in the earlier 2026-04-23 full-gate pass:
  - `.\build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `89/89`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - targeted WS6/WS7 proof reruns included:
    - `cmake --build build --config Release --target unit_tests`: passed
    - `ctest --test-dir build -C Release --output-on-failure -R "(MultiDelayCoreTest|CloudSeedCoreTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"`: passed, `44/44`
- Later 2026-04-23 visible-rack sprint verification from this same checkout:
  - `cmake -S . -B build`: passed
  - `cmake --build build --config Release --target unit_tests`: passed
  - `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
  - `ctest --test-dir build -C Release --output-on-failure -R "DaisyHost(Standalone|Render)Smoke"`: passed, `2/2`
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release`: passed
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release`: passed
  - direct `DaisyHostRender.exe` runs against temporary forward and reverse
    two-node rack scenarios: passed, with manifests recording the expected
    `boardId`, `selectedNodeId`, `entryNodeId`, `outputNodeId`, `nodes[]`, and
    `routes[]`
  - later same-day Braids hosted-app verification added:
    - `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release`: passed
    - checked-in `braids_smoke.json` render path: passed as part of the render smoke run
  - blocker observed in that pass: `DaisyHostUnitTestsRunner.exe` could
    disappear or become
    externally locked after link in this shell, so the rebuilt unit-test binary
    cannot currently be executed and the full host gate has not yet been rerun
    against the final visible-rack plus Braids code
- Resumed 2026-04-24 Braids verification from this same checkout:
  - `cmd /c build_host.cmd`: configured and rebuilt the Release app target set,
    then failed during full `ctest -C Release` because CTest could not launch
    the Release `DaisyHostUnitTestsRunner.exe` path
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(AppRegistryTest|BraidsCoreTest|DaisyBraidsCoreTest|RenderRuntimeTest)"`: passed, `27/27`
  - `ctest --test-dir build -C Debug --output-on-failure -E "DaisyHost(Standalone|Render)Smoke"`: passed, `110/110`
  - `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed
  - blocker observed in that pass: full Release `ctest` could not launch the
    Release unit-test runner in that shell, so that was not a fully green
    Release host gate
- Later 2026-04-24 rack-freeze resume diagnostics from this same checkout:
  - `.\build_host.cmd`: first exposed and then confirmed the fixed
    `build_host.ps1` PATH-normalization path; the wrapper now reaches the real
    configure/build/test loop
  - direct `probe.exe` creation in `build/test_bin/Release`: passed
  - safe cleanup of `build/test_bin/Release` and `build/unit_tests.dir/Release`:
    passed
  - `.\build_host.cmd`: rebuilt the full Release target set again, including
    `unit_tests`, then failed in `ctest -C Release` because the Release runner
    remained `resource busy or locked`
  - direct launch of the Release runner with a single-test filter: failed
    because the file was in use by another process
  - copied `DaisyHostRender.exe` executed successfully from
    `build/test_bin/Release`, isolating the issue to the Release unit-test
    runner rather than the output directory
  - attempted Defender exclusion for
    `DaisyHost/build/test_bin/Release`: failed due insufficient permissions
  - result: the canonical Release host gate is still not green, so the Daisy
    Field readiness line has not been reached
- Later 2026-04-24 wrapper-mitigation follow-up from this same checkout:
  - exact-name write probes proved `build/test_bin/Release` was not generally
    broken: `DaisyHostUnitTestsRunner.exe` was denied, while nearby names such
    as `DaisyHostUnitTests.exe` and `unit_tests.exe` were writable
  - scratch-directory checks proved the same friendly unit-test name was
    writable and runnable outside the original path
  - `build_host.ps1` now passes a fresh
    `-DDAISYHOST_UNIT_TEST_RUN_TAG=<timestamp>` into configure so each wrapper
    run uses a new Release payload path
  - `unit_tests` now builds as
    `build/unit_test_bin/<run-tag>/Release/DaisyHostTestPayload.bin`, and
    `ctest` launches it through `tests/run_unit_test_payload.py`
  - `cmd /c build_host.cmd` rebuilt the full Release target set again and then
    ran `ctest -C Release`; `DaisyHostStandaloneSmoke` and
    `DaisyHostRenderSmoke` both passed, but the remaining `110/112` unit tests
    failed because the launcher got `PermissionError` when reading the freshly
    linked Release payload
  - attempted Defender exclusion for `build/unit_test_bin` and
    `build/unit_test_run`: failed due insufficient permissions
  - result: the rack freeze gate is still blocked by a machine-level read lock
    on the freshly linked Release payload itself, and Daisy Field
    implementation has still not started
- Later 2026-04-24 Harmoniqs + VA Synth hosted-app verification from this same
  checkout:
  - red step:
    `cmake --build build --config Debug --target unit_tests` first failed after
    the new tests/CMake wiring because the `DaisyHarmoniqsCore`,
    `HarmoniqsCore`, `DaisyVASynthCore`, and `VASynthCore` surfaces did not yet
    exist
  - green targeted proof:
    `cmake --build build --config Debug --target unit_tests`: passed
  - direct Debug payload proof:
    `py -3 tests/run_unit_test_payload.py <fresh Debug payload> --gtest_filter="AppRegistryTest.*:DaisyHarmoniqsCoreTest.*:HarmoniqsCoreTest.*:DaisyVASynthCoreTest.*:VASynthCoreTest.*:RenderRuntimeTest.ProducesDeterministicOfflineRenderForHarmoniqs:RenderRuntimeTest.ProducesDeterministicOfflineRenderForVASynth"`:
    passed, `18/18`
  - current Release render-path proof:
    `cmake --build build --config Release --target DaisyHostRender`: passed
  - current Release render smoke:
    `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`:
    passed, now covering `multidelay`, `torus`, `cloudseed`, `braids`,
    `harmoniqs`, and `vasynth`
- Firmware note: no `make` rerun was required in this 2026-04-23 pass because
  the WS6/WS7 package stayed host-side; the last recorded firmware reference
  reruns remain the successful 2026-04-22 `patch/MultiDelay/` and `patch/Torus/`
  builds.
- Historical note: the prior full runtime verification recorded in local docs
  before this rerun was `2026-04-19`
- In this Codex shell, raw MSBuild commands still require a sanitized one-path
  environment because both `Path` and `PATH` are exported by default, but
  `build_host.cmd` now encapsulates that repair.

## Last Recorded Artifact Paths

The following paths were verified from this checkout on 2026-04-23:

- Launcher hub:
  - `DaisyHost/build/DaisyHostHub_artefacts/Release/DaisyHost Hub.exe`
- Render CLI:
  - `DaisyHost/build/Release/DaisyHostRender.exe`
- VST3 bundle:
  - `DaisyHost/build/DaisyHostPatch_artefacts/Release/VST3/DaisyHost Patch.vst3`
- Standalone app:
  - `DaisyHost/build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe`
- Host test target:
  - `unit_tests`
  - executable payload currently built as:
    `DaisyHost/build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
  - `ctest` currently launches unit cases through
    `tests/run_unit_test_payload.py`
  - historical note: the later TF12 pass supersedes this as current checkout
    truth with a 2026-04-26 `202/202` gate including the CLI CTest smoke
    entries

## Current Hosted Apps

- default app id: `multidelay`
- additional app id: `torus`
- additional app id: `cloudseed`
- additional app id: `braids`
- additional app id: `harmoniqs`
- additional app id: `vasynth`
- additional app id: `polyosc`

Current Phase 2 host behavior:

- DaisyHost instantiates hosted apps by stable `appId`
- the selected app persists in `HostSessionState`
- the processor/editor bind to the active app's metadata and patch bindings
- the processor now exposes a fixed five-slot DAW automation bank with stable
  ids `daisyhost.slot1` .. `daisyhost.slot5`
- those five slots rebind to the active app's top-ranked automatable canonical
  parameters without changing the saved slot ids
- `multidelay` and `cloudseed` now expose named MetaControllers through the
  shared menu/drawer path while canonical parameter ids remain the only saved
  truth
- session persistence remains canonical by app parameter id rather than DAW slot
  id
- `GetEffectiveHostStateSnapshot()` now exposes live canonical parameters,
  mapped automation slots, selected-node identity, node count, board/topology
  fields, active-node MetaControllers, node summaries, routes, CV state, gate
  state, and current audio-input state for processor/UI/debug tooling
- the live plugin and standalone app now run exactly two hosted nodes
  (`node0`, `node1`) with selected-node-targeted Patch controls, drawer/menu
  actions, CV/gate/test-input state, keyboard MIDI, and five-slot DAW
  automation
- live rack topology is currently limited to four audio-only presets:
  - `node0_only`
  - `node1_only`
  - `node0_to_node1`
  - `node1_to_node0`
- board creation now flows through a board-id-based factory seam with
  `daisy_patch` as the default board and `daisy_field` as a Field
  board-support package
- Field native controls are host-side only:
  - K1-K4 mirror the selected node's current Patch page bindings
  - K5-K8 map to the next four automatable selected-node parameters by
    `importanceRank`, with missing parameters disabled
  - CV1-CV4 and Gate In reuse the existing host CV/gate input paths
  - A1-B8 emit 16 chromatic MIDI notes from the selected node's current
    keyboard octave
- Field extended host surface support is also host-side only:
  - CV OUT 1-2 are derived monitor outputs that mirror the K5/K6 mapped
    parameters as `0..5V`
  - SW1/SW2 are momentary selected-app utility triggers
  - key LEDs, switch LEDs, Gate In LED, and Gate Out LED are derived indicators
  - snapshots and render manifests carry final Field surface evidence for
    automated checks
  - the rack topology, hosted-app DSP behavior, selected-node routing
    semantics, Field firmware, real hardware voltage output, mixed-board racks,
    and manual DAW/VST3 validation remain out of scope
- `HostSessionState` is now v5 with rack globals:
  - `boardId`
  - `selectedNodeId`
  - `entryNodeId`
  - `outputNodeId`

Current `CloudSeed` supported-app behavior:

- hosted app id: `cloudseed`
- shared portable wrapper: `DaisyCloudSeedCore`
- named MetaControllers:
  - `Blend`
  - `Space`
  - `Motion`
  - `Tone`
- performance page model:
  - `Space` = `Mix`, `Size`, `Decay`, `Diffusion`
  - `Motion` = `Pre-Delay`, `Damping`, `Mod Amt`, `Mod Rate`
- encoder/menu sections:
  - `Pages`
  - `Arp`
  - `Program`
  - `Utilities`
  - `Info`
- `Arp` controls:
  - `Enabled`
  - `Rate`
  - `Pattern`
  - `Target`
  - `Depth`
- arpeggiator behavior:
  - deterministic host-time parameter stepping over selected performance
    groups
  - `Space` target group = `Mix`, `Size`, `Decay`, `Diffusion`
  - `Motion` target group = `Pre-Delay`, `Damping`, `Mod Amt`, `Mod Rate`
  - stored canonical values stay unchanged while effective values step over
    time for audio/render
- utilities currently expose:
  - `Bypass`
  - `Clear Tails`
  - `Randomize Seeds`
  - `Interpolation`
- DAW automation priority for the first five slots is:
  - `mix`
  - `size`
  - `decay`
  - `diffusion`
  - `pre_delay`

Current `Braids` supported-app behavior:

- hosted app id: `braids`
- shared portable wrapper: `DaisyBraidsCore`
- MIDI-first trigger source with optional `gate_in_1` rising-edge strike alias
- mono synth voice duplicated to stereo outputs 1/2
- no host audio input in v1
- performance page model:
  - `Drum` = `Tune`, `Timbre`, `Color`, `Model`
  - `Finish` = `Resolution`, `Sample Rate`, `Signature`, `Level`
- model subset currently ships:
  - `Kick`
  - `Snare`
  - `Cymbal`
  - `Drum`
  - `Bell`
  - `Filtered Noise`
- encoder/menu sections:
  - `Pages`
  - `Model`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Randomize Model`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `model`
  - `tune`
  - `timbre`
  - `color`
  - `signature`

Current `Harmoniqs` supported-app behavior:

- hosted app id: `harmoniqs`
- shared portable wrapper: `DaisyHarmoniqsCore`
- additive mono synth voice duplicated to stereo outputs 1/2
- MIDI-first trigger source with optional `gate_in_1` retrigger alias
- no host audio input in v1
- performance page model:
  - `Spectrum` = `Brightness`, `Tilt`, `Odd/Even`, `Spread`
  - `Envelope` = `Attack`, `Release`, `Detune`, `Level`
- stateful expert spectrum lanes currently ship:
  - `harmonic_1` through `harmonic_8`
- encoder/menu sections:
  - `Pages`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Init Spectrum`
  - `Randomize Spectrum`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `brightness`
  - `tilt`
  - `spread`
  - `odd_even`
  - `attack`

Current `VA Synth` supported-app behavior:

- hosted app id: `vasynth`
- shared portable wrapper: `DaisyVASynthCore`
- fixed seven-voice polyphonic synth with MIDI-first triggering
- optional `gate_in_1` rising-edge trigger alias
- stereo output on Patch outputs 1/2 with toggleable `Stereo Sim`
- no host audio input in v1
- performance page model:
  - `Osc` = `Mix`, `Detune`, `Osc 1`, `Osc 2`
  - `Filter` = `Cutoff`, `Resonance`, `Env Amt`, `Level`
  - `Motion` = `LFO Rate`, `LFO Amt`, `Attack`, `Release`
- encoder/menu sections:
  - `Pages`
  - `Oscillators`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Init Patch`
  - `Stereo Sim`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `osc_mix`
  - `detune`
  - `osc1_wave`
  - `osc2_wave`
  - `filter_cutoff`

Current `PolyOsc` supported-app behavior:

- hosted app id: `polyosc`
- shared portable wrapper: `DaisyPolyOscCore`
- host-side import of `patch/PolyOsc` behavior:
  - three oscillators rendered to Patch outputs 1-3
  - mixed oscillator output rendered to Patch output 4
  - stereo host/render output duplicates output 4 through `mainOutputChannels`
- no host audio input, MIDI input, gates, CV inputs, or firmware adapter in this
  pass
- Patch control model:
  - K1-K3 = oscillator 1-3 frequency controls
  - K4 = global frequency offset
  - encoder = waveform selection across `Sine`, `Triangle`, `Saw`, `Ramp`, and
    `Square`
- Field host-surface model:
  - K1-K4 mirror the Patch controls
  - K5 maps to `waveform`
  - K6-K8 and SW1/SW2 remain unavailable for PolyOsc
- DAW automation priority for the first five slots is:
  - `osc1_freq`
  - `osc2_freq`
  - `osc3_freq`
  - `global_freq`
  - `waveform`

Current Phase 3 render behavior:

- `DaisyHostRender` loads scenario JSON by `appId`
- each render run resets app state to a fixed seed, restores canonical
  parameters, replays a timeline, and renders offline
- legacy single-app scenarios remain valid, while optional `nodes[]` and
  `routes[]` contracts plus rack-level board/selection/topology fields now
  support forward and reverse two-node audio-chain proofs
- each run writes `audio.wav` plus `manifest.json`
- multi-node manifests now include recorded `boardId`, `selectedNodeId`,
  `entryNodeId`, `outputNodeId`, per-node summaries, and recorded routes while
  the top-level app fields continue mirroring `node0`
- `training/render_dataset.py` expands sweep jobs into multiple run folders and
  writes `dataset_index.json`
- checked-in smoke scenarios now include:
  - `multidelay_smoke.json`
  - `torus_smoke.json`
  - `cloudseed_smoke.json`
  - `braids_smoke.json`
  - `harmoniqs_smoke.json`
  - `vasynth_smoke.json`

Current hub behavior:

- `DaisyHost Hub` is a small front door for:
  - `Play / Test`
  - `Render`
  - `Train`
- v1 board selection currently exposes `daisy_patch` and `daisy_field`
- the hub persists its own launcher profile separately from plugin/session
  state
- `Play / Test` writes a small startup request so the standalone host opens the
  selected app directly

## Final Control Map For The Active Refresh

Agreed Patch control hierarchy for the current `0.2.0` refresh:

- `CTRL 1` = dry/wet mix
- `CTRL 2` = primary delay control
- `CTRL 3` = secondary delay control
- `CTRL 4` = feedback
- `CV 1` = tertiary delay control
- `ENC 1` rotate = menu navigation / value edit
- `ENC 1` push = menu open, enter edit, confirm, or exit edit

For the current `MultiDelayCore`, the delay importance order is:

- primary = old delay 1 target
- secondary = old delay 2 target
- tertiary = old delay 3 target

Shared behavior contract:

- every DSP parameter must also be editable from the OLED menu
- if a knob, `CV 1`, or the menu all target the same parameter, `last touch
  wins`

## Menu And Input Targets For The Refresh

Shared top-level menu structure:

- `Params`
- `Input`
- `MIDI`
- `Tracker`
- `About`

`Params` must expose:

- `Dry/Wet`
- `Delay 1`
- `Delay 2`
- `Feedback`
- `Delay 3`

Input/test-source target behavior for the refresh and renderer:

- standalone default = `Host In`
- alternate sources = `Sine`, `Triangle`, `Square`, `Saw`, `Noise`, `Impulse`

Version/change tracking target behavior:

- `About` page mirrors the current version and recent changelog bullets
- `CHANGELOG.md` is the canonical written release history

## Current Notes

- The last fully green `build_host.cmd` host-gate rerun from this checkout is
  now the 2026-04-26 TF12 verification/build hardening `202/202` pass.
- Older `159/159`, `168/168`, `196/196`, and `197/197` counts remain useful as
  dated historical ledger evidence, but they are not the current checkout gate.
- The standalone smoke harness now tolerates slower Windows process-path
  discovery by using a wider process-query timeout before declaring a false
  launch timeout.
- Workstreams 4 and 5 are now source-backed in this checkout:
  - DaisyHost exposes a fixed five-slot DAW automation bridge that maps stable
    slot ids onto the active app's canonical automatable parameters
  - the processor exposes an in-process effective-state snapshot/readback API
    for parameters, mapped slots, CV, gate, and audio-input state
- Workstream 6 is now source-backed in this checkout:
  - `multidelay` and `cloudseed` both expose named MetaControllers that write
    back into canonical parameters and are surfaced through the mirrored
    menu/drawer path
  - `cloudseed` and `braids` are now treated as first-class supported apps
    rather than only pilots
- Visible Workstream 7 rack runtime is now source-backed, build-backed, and
  smoke-backed in this checkout:
  - `HostSessionState` now serializes `version 5` sessions with rack globals,
    `node` records, and `route` records while keeping legacy single-node
    sessions readable
  - the live plugin/standalone now run exactly two hosted nodes with
    selected-node-targeted controls and four audio-only topology presets
  - the editor now shows a visible rack header with per-node app selectors,
    selected-node highlighting, topology controls, Field board-support shell
    rendering, interactive Field K1-K8 controls, and momentary Field A1-B8 keys
  - `daisy_field` now has host-side Field native controls plus Field extended
    host surface support: derived CV OUT 1-2 monitor values, SW1/SW2 momentary
    utility triggers, and key/switch/gate LEDs in snapshots, render manifests,
    smoke, and the panel UI
  - Field refinement closeout adds Hub startup-request launch planning,
    selected-node Field surface render/smoke evidence, and board-aware
    selected-node mirror copy
  - Field ergonomics/UI cleanup adds explicit Patch-page knob parameter ids for
    hosted apps, makes Field K5-K8 avoid duplicate K1-K4 targets, and lets the
    Field editor resolve knobs, keys, and switches through board-profile target
    metadata
  - `field/MultiDelay` is the first flash-verified Field firmware adapter, and
    adapter-pipeline v0 can generate the build/QAE-verified
    `field/MultiDelayGenerated` adapter from a shared-core spec
  - full manual Field hardware validation, generated-adapter flashing, broad
    Field firmware parity, real hardware voltage-output measurement,
    Field-specific app ergonomics, mixed-board rack behavior, arbitrary
    firmware import, and manual DAW/VST3 validation remain deferred
  - board creation now flows through a board-id factory seam instead of
    hardcoded Patch construction in the processor path
  - render scenario and manifest contracts now accept rack globals plus
    `targetNodeId` for multi-node non-ID-scoped events
  - the render runtime now proves forward and reverse two-node audio chains
    while rejecting cross-node CV, gate, and MIDI routes with clear errors
- `tests/run_smoke.py` now provides the direct-entrypoint smoke harness:
  - `DaisyHostStandaloneSmoke` launches `DaisyHost Patch.exe` with
    `--board daisy_patch --app torus`, requires a crash-free 5s warmup window,
    then terminates the process cleanly; the process-path query now has a
    wider timeout budget for slower shells
  - `DaisyHostRenderSmoke` runs `DaisyHostRender.exe` against the checked-in
    `multidelay`, `torus`, `cloudseed`, `braids`, `harmoniqs`, `vasynth`,
    `polyosc`, Field board-support shell, Field native controls, Field extended
    surface, Field selected-node surface, and Field PolyOsc surface scenarios,
    verifies `audio.wav` plus
    `manifest.json`, checks repeated `multidelay` `audioChecksum`
    determinism, and validates final Field surface evidence for CV OUT, SW,
    LED state, selected-node Field surface state, and PolyOsc K5 waveform
    mapping
- `cloudseed` continues to wrap `third_party/CloudSeedCore` through a portable
  `DaisyCloudSeedCore`, and page changes refresh the JUCE processor's active
  patch bindings so DaisyHost control labels and control ids follow the active
  `Space` / `Motion` page instead of staying stale after menu navigation
- `../third_party/CloudSeedCore/DSP/RandomBuffer.cpp` now has a local MSVC-safe
  dynamic-buffer fix because the upstream VLA form does not compile in this
  Windows C++17 build
- `build_host.cmd` and `build_host.ps1` now provide the preferred local host
  gate entrypoint for this checkout and encapsulate the `Path` / `PATH`
  normalization needed by raw MSBuild commands here.
- `tools/write_vst3_manifest.ps1` now drives VST3 manifest generation from a
  PowerShell helper invocation because the direct MSBuild / `cmd` launch path
  for `juce_vst3_helper.exe` failed in this checkout with a `LoadLibraryW`
  path-resolution error.
- The standalone package uses the compact DaisyHost icon asset through
  `ICON_SMALL`.
- The standalone input-mute warning banner is suppressed in the app UI while
  the underlying host-input mute behavior remains unchanged.
- `patch/MultiDelay` requires a Windows-friendly clean rebuild when stale host
  objects are present: remove `build/` with PowerShell, then run `make`.
- `patch/Torus` also requires a PowerShell `build/` removal for clean rebuilds
  in this Windows shell because `make clean` delegates to `rm`.
- The Torus pilot inside DaisyHost is intentionally a DaisyHost-native wrapper
  with Torus/Rings semantics and menu/control assignment behavior. It does not
  compile the original `patch/Torus` firmware entrypoint or Daisy UI layer into
  the host.
- Phase 3 scenario/dataset examples live under:
  - `training/examples/multidelay_smoke.json`
  - `training/examples/torus_smoke.json`
  - `training/examples/cloudseed_smoke.json`
  - `training/examples/braids_smoke.json`
  - `training/examples/harmoniqs_smoke.json`
  - `training/examples/vasynth_smoke.json`
  - `training/examples/polyosc_smoke.json`
  - `training/examples/field_cloudseed_shell_smoke.json`
  - `training/examples/field_vasynth_native_controls_smoke.json`
  - `training/examples/field_extended_surface_smoke.json`
  - `training/examples/field_node_target_surface_smoke.json`
  - `training/examples/field_polyosc_surface_smoke.json`
  - `training/examples/dataset_job_example.json`
- `WORKSTREAM_TRACKER.md` now holds the dedicated post-WS7 portfolio tracker,
  and the same tracker content is mirrored into `PROJECT_TRACKER.md`.
- Full DAW-side VST3 validation remains intentionally deferred until a
  post-Workstream-7 manual pass; no automated DAW load test is wired into this
  workspace yet.

## Final Verification Gate Before Calling The Plugin Complete

Use the following commands for the next full rerun:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

If shared-core or firmware-facing code changed, also run:

```sh
make
```

from `patch/MultiDelay/` and/or `patch/Torus/` as appropriate.

## Last Recorded Render Checks

The following DaisyHostCLI render-path checks were last recorded as executed on
2026-04-26:

```sh
DaisyHost/build/Release/DaisyHostCLI.exe render DaisyHost/training/examples/multidelay_smoke.json --output-dir DaisyHost/build/cli_smoke/tf12_multidelay --json
DaisyHost/build/Release/DaisyHostCLI.exe smoke --mode render --build-dir DaisyHost/build --source-dir DaisyHost --config Release --json
```

The CLI render produced checksum `c9c3f665e6a0dd2b`.

The following historical manual render commands were last recorded as executed
on 2026-04-19:

```sh
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/multidelay_smoke.json --output-dir DaisyHost/training/out/multidelay_smoke
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/torus_smoke.json --output-dir DaisyHost/training/out/torus_smoke
py -3 DaisyHost/training/render_dataset.py DaisyHost/training/examples/dataset_job_example.json
```

## Last Recorded Manual Checks

Automated note:

- direct-entrypoint standalone startup smoke and render smoke were rerun on
  2026-04-23 through `tests/run_smoke.py`
- the remaining checks below are still manual / historical unless separately
  rerun

The following manual checks were last recorded on 2026-04-19:

- standalone launch stability: passed via 5s smoke launch
- VST3 load in a DAW: not verified in that session
- visible standalone icon: not visually rechecked after the final build in that
  session
- `Host In`, `Sine`, `Saw`, `Noise`, and `Impulse`: build/test covered; manual
  audio pass not repeated after the final UI patch
- app selection and app restore:
  - host/session serialization covered by tests
  - no manual GUI click-through for `multidelay` -> `torus` -> restore was run
    in that session
- render CLI:
  - `multidelay` scenario: rendered successfully
  - `torus` scenario: rendered successfully
  - repeated `multidelay` scenario: checksum matched on rerun
  - dataset sweep job: expanded and rendered successfully
- hub:
  - `DaisyHost Hub.exe`: built successfully
  - manual GUI click-through not run in that session
- mouse access to every visible control: editor wiring implemented; not
  exhaustively re-clicked after the final build
- computer keyboard MIDI: previously verified in-session
- external MIDI note input: previously verified in-session
- CC learn: previously verified in-session
- MIDI tracker updates: previously verified in-session
- encoder menu and side drawer sync: implemented in the refreshed editor;
  manual recheck still recommended
- all five DSP parameters editable from both menu and live controls: shared core
  tests pass; final end-to-end manual pass still recommended
- host and Patch firmware menu behavior aligned at the feature level: shared
  core plus firmware adapter build verified
