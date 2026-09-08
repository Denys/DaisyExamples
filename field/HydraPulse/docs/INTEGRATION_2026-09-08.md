# DaisyExamples integration - 2026-09-08

Status: host and ARM qualification PASS locally and on branch CI. Both firmware flashes VERIFIED on two different Seed processors. Diagnostic listening FAILED/INCONCLUSIVE on the first Seed; operator reports working instrument output, sequencer, some sounds and controls on the replacement Seed. Full musical and stereo qualification remains incomplete.

## Inspected inputs and preservation

- Target master: `041bfcbb7975316606ece8f1ccbacbab9c0e1482`.
- Branch: `codex/hydrapulse-field-integration`, in an independent clone under `.worktrees/hydrapulse-integration-20260908`.
- ZIP: `HydraPulse_DaisyExamples_0.2.0_candidate_2026-09-07.zip`.
- Installer preflight: 151 new files, zero collisions; all payload hashes checked before installation.
- Original workspace: 150 candidate files identical to ZIP, workflow missing; extra delivery attachments left untouched.
- Standalone `Denys/HydraPulse-Field` main: `184db5473aea258f660604d791d305f7061671fc`, inspected through the authenticated GitHub connector. It contains newer R0 integration, a separate FieldTruth implementation, tests and research. Nothing there was changed or copied over. `references/r0` means the original supplied 29-file archive, not a mirror of current standalone main.
- Root libDaisy gitlink retained: `85172e2b5c9abea1bf56dcd5d2c7b3f5717eb66b`; LICENSE blob `1c3a251694f1d1842c3f114508d36ca63cddf275`.
- Nested googletest: `f5e592d8ee5ffb1d9af5be7f715ce3576b8bf9c4`.
- No root checkpoint or HydraPulse entry in LATEST_PROJECTS was available. Nearest project documents and live source were used.

## Candidate deviations

1. Reuse the existing root libDaisy pin instead of adding candidate-private `cc146d5065dd8286078a662e2830bf820c37a612`. Root gitlinks and the primary dirty library checkout are unchanged.
2. Use the pinned BSP's `dsy_gpio_write` Gate Out API; give the adapter stub the same signature and LED overloads.
3. Make fixed-width integer `std::max` calls explicit, because ARM newlib uses a different underlying `uint32_t` type than host GCC.
4. Use floating-point zero for LED brightness to avoid the BSP's integer/float overload ambiguity.
5. Format active C++ with CI's clang-format 10.0.1; replace unsupported SortIncludes enum with boolean. Generated presets have formatter guards. Exclude only the immutable R0 archive from the root formatting workflow.
6. Add direct BSP includes to the firmware translation units. QAE's per-file analysis does not follow adapter includes.

7. Use `-Os` for application code: the instrument at `-O2` exceeded internal flash by 3632 bytes. Both images now link below 128 KiB without a bootloader/layout change; final forced-build manifest verified.
8. Override the local SWD `program` recipe to use the correct `build-truth` or `build-beat` ELF; upstream libDaisy remains unchanged.
9. Force-build helpers now create and mark directory targets old (`-o`) while forcing all objects with `-B`; the pinned upstream `mkdir build` recipe otherwise fails on a second qualification run.
10. Add `tools/qualify_host.py` to run and log all host gates. Teach logger-width tests to accept formatter-inserted whitespace before the format string.
11. Repair a Mermaid sequence-note semicolon parse error and replace the clock diagram layout with an equivalent flowchart to avoid overlapping edge labels. Both Markdown fences and `.mmd` sources match.
12. Change only the OLED fault counter prefix from `E` to `#`, avoiding QAE's regex false positive on an escaped percent followed by E. Formatting remains integer-only.

13. Limit final Git status collection to HydraPulse with unrelated submodules ignored. A complete ARM build had reached artifact creation but the old repository-wide status scan timed out before writing its manifest. The failure transcript is preserved; only a successful manifest qualifies the final images.

14. Add Git attributes preserving archived R0 and raw acquisition log bytes across platform checkouts. Raw logs are binary-diff entries so recorded trailing whitespace remains intact.

15. Match the root workflows' `actions/checkout@v7` and default credential persistence for the parent checkout. Early cleanup with `persist-credentials: false` fails on the repository's pre-existing orphan `.tmp/ai-book-sparse` gitlink, under both v4 and v7, before tests run. The ephemeral CI token has only contents-read permission; independent libDaisy checkout still removes credentials immediately. No unrelated submodule metadata was changed.

## Current software evidence

- `tools/qualify_host.py`: all 15 commands returned zero, with per-command logs and a JSON index.
- Release (GCC 13.3): 14/14 CTest PASS; UBSan (GCC 13.3): 14/14 PASS; ASan+UBSan (Clang 18.0): 14/14 PASS.
- Python: 38/38 PASS; repository/provenance, generated presets and 29-file R0 preservation PASS.
- Host deterministic stereo render comparison PASS. No host render is a Field recording.
- Upstream libDaisy host tests: 151/151 PASS with GCC 13.3. Clang 18 first rejected an existing GNU variable-length-array extension under upstream -Werror; no upstream source or flags were changed for the GCC run.
- QAE: zero errors, seven advisory warnings. Two NO-START-ADC reports are per-file false positives: `InitHardware()` starts ADC before `StartAudio()`. Remaining heap/iostream reports concern host tests and the renderer.
- clang-format 10.0.1: all 37 active C++ files match. Immutable R0 sources were not formatted.
- Mermaid 11.12.2 / headless Edge: all eight diagrams rendered to SVG/PNG with no page errors; all eight screenshots inspected for clipped or overlapping labels. The corrected clock flowchart is readable. Desktop intrinsic-size renders only; no mobile layout claim.
- ST-Link identified an STM32H74x/75x Cortex-M7 with a single 128 KiB internal-flash bank. Internal flash backup retained locally before any write: 131072 bytes, SHA-256 `80971a0ee9a0e019ac692308618a8300a1861e5a78102033f3162a31272dc95e`. Recovery image is not published.

## ARM and flash evidence

- Final `tools/build_target.py` completed successfully after rebuilding the library and both images. See `evidence/arm-build-2026-09-08.json` for exact input, compiler, dependency and ELF/BIN/MAP SHA-256 records.
- Runtime source digest: `24e035a726a12d2ad72f8b69d1491a3ad17267acbbcf1a38135e8937bd8250cc`.
- FieldTruth (`HydraPulseTruth.bin`): 107280 bytes; SHA-256 `72a21e32621174f5d66cbcade3711b21d5810711813b27fc405c696e48bdab60`.
- HydraPulse (`HydraPulseBeat.bin`): 122320 bytes; SHA-256 `6b0d5b518b20ad57720755ee6330b684b2e905704ebaa35d906225a9173dc458`.
- Compiler: GNU Arm Embedded 13.2.1, Ubuntu package; Cortex-M7 hard-float; BOOT_NONE. Application `-Os`, library `-O3`, both finite-math qualification flags and default-width enums.
- ELF sections (text/data/bss bytes): Truth 104252/2988/86116; Beat 113492/8788/85196. Linker usage: Truth FLASH 107280, SRAM 71160, RAM_D2 17224; Beat FLASH 122320, SRAM 76040, RAM_D2 17224; BACKUP_SRAM 12 each. Neither application allocates SDRAM or QSPI sections. The linked SRAM includes reserved heap/stack; no measured stack high-water claim.
- Linker emits enum attribute warnings from GNU runtime objects. Actual application and libDaisy translation units use int-width enums and application static assertions enforce the public BSP MIDI enum width. The warning is retained in logs. Newlib nosys stubs also warn about unsupported POSIX I/O; no such I/O is performed in audio callbacks.
- Diagnostic ELF matched current runtime sources and successful manifest immediately before flashing.
- Native Windows OpenOCD 0.12.0 with ST-Link serial `0020000A5553500920393256`: **Programming Finished / Verified OK / Resetting Target**, exit 0. The native tool executes the same inspected APP-specific ELF recipe because USB/SWD is attached to Windows, while compilation runs in WSL.
- Instrument flash: VERIFIED on the replacement Seed after the operator explicitly requested loading HydraPulse despite the unresolved diagnostic audio result. This is an operator-authorized diagnostic continuation, not a passed P0 promotion. Basic musical output is now operator-reported on the replacement Seed; full P0 and all-gesture audio acceptance remain incomplete.

## Initial hardware runtime capture

The flashed diagnostic enumerated as Daisy USB CDC COM5 (`0483:5740`) and emitted
`build=24e035a726a1`, 48 kHz / 48-sample blocks, CPU 400 MHz and tick clock 200 MHz.
A 45-second capture reports max load 28 per mille, zero overruns and zero late starts.
Raw pressed indices captured: 0, 1, 2, 8, 9. This is not all sixteen physical keys.
CV/Gate output commands were zero; audio input peaks were approximately 0.70 full scale
in both channels. Input telemetry alone does not prove analog output or passthrough quality.
The initial serial backlog contains one malformed record and a cumulative drop count of
857. Preserve this capture; do not classify it as clean complete hardware validation.
No intentional MIDI stimulus, knob endpoint sweep, all-key mapping or operator listening
result was recorded during the initial captures. The later operator report and replacement-Seed flash are recorded below.

A second 30-second acquisition discarded the connection backlog before capture.
It contains 0 malformed records, with callback delta 29560 and drop delta 0.
Overrun and late-start counters remained zero. The cumulative drop counter was 3058 at capture start and 3058 at its end; disconnected time is not a measured clean interval.
Raw logs and derived summaries for both acquisitions are retained without byte normalization.

## Operator report and replacement Seed

The operator reported a responsive OLED, key LEDs responding on press, and both switches affecting the display. Knob LEDs did not change brightness; inspection confirms FieldTruth drives only key LEDs, so knob-dependent LED brightness is not implemented. This does not prove all eight analog knob endpoints.

The operator heard either no audio or a whistle with FieldTruth, then replaced the Seed processor. An older synth on the replacement reportedly did not whistle. The cause is unresolved; input passthrough routing, the first processor and the firmware have not been discriminated by measurement. Do not mark diagnostic audio PASS.

The operator explicitly requested loading HydraPulse on the replacement. SWD confirmed a different processor UID (`002e0044 33305110 39383339`, versus the original `001c003b 3330510f 39383339`), still STM32H74x/75x with 128 KiB flash. A separate full internal-flash backup was retained locally before writing: 131072 bytes, SHA-256 `59a8481b89a5f621eb6f8a14318227e1fccdee24824bb562b9ed1e2f7d1735a7`. Neither recovery image is published.

HydraPulseBeat's local qualified ELF was programmed, verified and reset through ST-Link, exit zero. The replacement enumerated on COM7 and emitted `app=beat build=24e035a726a1`, 48 kHz / 48 samples, CPU 400 MHz. A 55-second capture had no malformed records, max reported load 123 per mille, zero faults/overruns/late starts and zero MIDI/snapshot queue drops. These counters do not establish audible behavior or physical gesture coverage. The subsequent operator listening report is recorded below; the capture itself does not verify listening results.

## Subsequent operator smoke result

After the replacement-Seed HydraPulse flash, the operator reported that output now works correctly, the sequencer works, and at least some sounds work. SW1/SW2, keys, key LEDs, knobs and volume were reported working. This is a positive basic instrument smoke result reported by the operator, not a recorded or independently measured audio test.

The operator suspects the previous processor was faulty. That is plausible but unproven: the earlier listening test used FieldTruth passthrough and the successful later test uses HydraPulse synthesis on a different Seed. A controlled comparison or electrical measurement is still needed to identify the cause of the original whistle.

Knob LEDs are not implemented in either firmware image. Both display routines first clear all LED channels, then populate only key LEDs; the instrument additionally sets the two switch LEDs. No knob LED brightness is written. Their dark state is therefore expected firmware behavior, not evidence that the knobs or their LEDs are defective.

Still unverified: all four voices individually; repeated Play/Stop and decay-to-silence; detailed click/DC/clipping checks; each musical gesture (accent, voice select, A/B, Fill, mute and Panic); measured stereo channel/routing behavior; all sixteen raw key mappings and all eight knob endpoints; full CV/Gate/MIDI P0 measurements. Do not infer these from the general positive smoke report.

## Remote software evidence and source identity

At source commit `fdd22f363b04447ceb92c2a2774ae9d96edc54df`, the [branch qualification run](https://github.com/Denys/DaisyExamples/actions/runs/34232758870) passed all three host modes and both ARM builds. [Build All](https://github.com/Denys/DaisyExamples/actions/runs/34232764124) and [Fix Style](https://github.com/Denys/DaisyExamples/actions/runs/34232764041) passed on the PR. Subsequent changes to this evidence report do not change runtime source.

All six downloaded CI ELF/BIN/MAP artifacts matched their manifest sizes and SHA-256 values. See `evidence/ci-arm-build-2026-09-08.json`. Local and CI manifests differ in the raw line endings of `src/app/Presets.generated.h`: CRLF locally and LF in Git. All 22 build inputs are identical after newline normalization; `evidence/commit-source-bridge-2026-09-08.json` records that comparison. This changes the embedded source ID (`24e035a726a1` local, `41e814118b1e` CI). For each application, the two BIN files are byte-identical after replacing only that embedded ID; ELF/MAP also carry build-directory/debug paths. Keep the local manifest for the images actually flashed; do not substitute CI hashes for them.

## Earlier attempts retained for traceability

- Candidate-after-install Python: 38/38 PASS.
- Candidate-after-install GCC 13.3 Release: 14/14 CTest PASS.
- GCC 13.3 UBSan: 14/14 CTest PASS.
- Deterministic host sonic scenarios: PASS for two runs; these are not Field recordings.
- First ARM qualification: libDaisy compiled with GCC ARM 13.2.1; firmware failed on the target-specific issues listed above. Failure transcript retained under `artifacts/2c2ca122dc96/BOOT_NONE/build.log`.
- First Clang ASan+UBSan link failed because libclang-rt was absent. Runtime installed; rerun required.
- QAE first scan: 3 errors, 7 warnings. Two missing-direct-include errors and one regex false positive interpreting the escaped percent before `E` as float printf. No float printf support was added.
- ST-Link USB serial `0020000A5553500920393256` detected. Daisy USB CDC not yet detected. No flash, runtime, physical control or analog audio result is implied.

Raw run logs and renders are under `artifacts/integration/`. `PACKAGE_MANIFEST.json` is the original ZIP inventory, not a hash assertion about the adapted working tree; run `install_additive.py` only against the untouched extracted candidate. Current source identity and image hashes are recorded by the final ARM manifest. No `dist/` binaries are regenerated by this task: the repository's generic dist helper searches `build/`, while these two deliberately separate images are uploaded by HydraPulse CI.
