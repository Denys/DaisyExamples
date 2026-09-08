# HydraPulse Field: foundation review and implementation plan
Date: 2026-09-07. Candidate version: 0.2.0-dev. Target: Denys/DaisyExamples, field/HydraPulse.
This plan is followed by implementation in this package, not a request for another planning session.

## 1. Preserve and establish the baseline
Verify the supplied R0 ZIP checksum. Preserve every one of its 29 files under references/r0.
Inspect clock, pickup, sequencer, tests, manifests and existing CI before changing them.
Use a new additive field/HydraPulse folder. Do not delete Denys/HydraPulse-Field or rewrite history.
On the connected Codex checkout, inspect the actual target HEAD/tree, nearest AGENTS.md, root README,
.gitmodules, CI scripts, build configuration and destination conflicts before applying the package.
The old remote SHA in the conversation is historical evidence, not a current HEAD assertion.

## 2. Audit boundaries before features
Check invalid numeric inputs, retuning semantics, sample timing, callback ownership,
cross-context communication, key mapping, mute/retrigger transitions and actual QA coverage.
Attach every material finding to a failing test or a specific absent implementation.
Retain the tested Pattern16 and StepSequencer interfaces; repair clock/pickup input handling.
Do not confuse a provenance validator's string checks with upstream verification.

## 3. Write the system reference before integration
Produce individually editable Mermaid files and companion explanations for system context,
firmware ownership, audio algorithms, clock/transport, control gestures, Field I/O, preset
transactions and verification. Use actual source symbols and make deferred features explicit.
Review diagrams against the resulting implementation and provide a renderer command for Codex.

## 4. Implement reusable primitives and host tests
Keep all musical/DSP logic independent of libDaisy. Implement finite ramps, deterministic noise,
a sine lookup, bounded AD envelopes, a swing-aware integer clock, fixed-capacity SPSC queues,
four voice algorithms, original tutorial presets and the direct-control state machine.
Test invalid inputs, finite outputs, decay, replay, schedule error, transport, bank transitions,
overflows, held-button gestures, parameter pickup and no-allocation processing.
Do not add a generic plugin/browser/modulation ecosystem merely for hypothetical portability.

## 5. Implement permanent P0 diagnostics
Separate firmware target: passthrough; keyboard edge/seen masks; knob/CV reporting;
Gate In counts; MIDI receive counts; manual DAC code selection; outputs normally off.
Release-to-arm then hold SW1 for output tests; SW2 alone for LED/OLED test.
Instrument the callback with the Seed testpoint and libDaisy CpuLoadMeter.
Label timer load as callback-body instrumentation, not total interrupt occupancy.
Preserve cold-start boot metadata for later serial connection; do not infer electrical reset cause.

## 6. Implement Beat Core as a bench candidate
Four synthesized voices: Hammer, Crack, Steel and Arc. Three 16-step banks: A, B and Fill.
One home screen; hold Shift for direct actions; release returns home. No persistent menu stack.
Four common sound controls, fixed tempo/swing/drive/master knobs, soft pickup after selection.
A/B changes at the next bar, momentary Fill at the next step, smooth mutes and panic.
Eight original tutorial presets, from blank to dense. Preset browsing never loads automatically.
The user's new instruction authorizes producing this candidate without waiting for P0 hardware
results. It does not authorize marking P0 passed: performance deployment still follows P0.

## 7. Use verified Daisy infrastructure, avoid collateral changes
Use libDaisy DaisyField BSP, core/Makefile and CpuLoadMeter. Respect GNU++17 selection explicitly.
Keep original DSP native-testable; DaisySP is not required by these particular algorithms.
Use the target repo's build/style/test tools only after inspecting their actual versions and scope.
Use a private per-project dependency checkout rather than repinning siblings' submodules.
Generate truth and beat binaries into distinct build directories. Never auto-flash in CI.
DVPE is optional: omit .dvpe until serialization, block registry and export round-trip are verified.

## 8. Validate and deliver
Execute host GCC/Clang tests, sanitizers where supported, waveform scenarios, source validators,
package checksum checks and an additive patch application test.
Run the real ARM compiler and libDaisy linker in Codex; preserve ELF/BIN/MAP, toolchain identity,
dependency SHAs and build logs. Fail clearly if compiler, libraries or supported flash size are absent.
Render Mermaid and run relevant upstream regression tools in that checkout.
A host peripheral stub only checks application syntax/contract; it is not an ARM build.
Deliver code, manual, preset lessons, test instructions, review and measured local result files.
Remote publishing requires a working authorized connector or authenticated Codex git session.
No such route is exposed in this authoring run, so remote writes are not performed.

## 9. Hardware and musical acceptance
After P0 flash, measure all physical controls, CV/Gate electrical behavior, calibrated audio,
MIDI input and timing under display/LED/serial load. Freeze measured control mapping.
After Beat flash, test each tutorial lesson, 60-minute dense performance, rapid retriggers,
transport/mute/preset changes, disconnected USB, queue pressure and near-endpoint controls.
Record worst observed callback time; initial proposed target is <50% of the 1 ms period,
with zero observed deadline overruns. This is an acceptance target, not a measured result.
Use calibrated loopback and listening to assess clicks, aliasing, DC, headroom and voice clarity.
No hardware test can be manufactured by desktop compilation.

## 10. Deferred growth with explicit entry conditions
After P0/P1 acceptance: persistent storage with power-fail tests, 64-step paging, additional three
voices, controlled destruction, then timing/clock integration and the stereo external lane.
Add complex locks/ratchets/probability only after A/B/Fill and direct operation remain usable.
Keep Hydrasynth-specific MIDI/CV/audio integration deferred. Do not copy manual patch settings
or commercial pattern-book content into presets.
