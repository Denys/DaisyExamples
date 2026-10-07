# POD MultiDelay demonstrator v0.1

Status: **HOST_TEST_PASS / ARM_BUILD_PASS / POD_DIGITAL_SCREEN_PASS / NORMAL_IMAGE_FLASH_VERIFIED**.
Primary lane: Delay. Prepared 2026-09-09 for the explicitly selected Daisy Pod.
This is a local integration candidate, with original mono DIGI and a source-linked
DaisyExamples reference engine for TAPE, MOD, REV and FREEZE. It does not replace
DAFX product source or accept new product-level algorithm/transition decisions.

## Contents and build

`build/Pod_MultiDelay.bin` is the BOOT_NONE image; ELF and MAP accompany it.
`Pod_MultiDelay_v0_1.zip` packages the adapter, tests, documentation, dependency
fingerprints and build image. External source trees and libraries are referenced,
not copied. The zip is a local engineering handoff, not a distribution release.

From this directory in Windows PowerShell:

```powershell
.\verify.ps1
py -3 -B .\package.py
```

Host compiler: `C:\msys64\ucrt64\bin\g++.exe` (GCC 14.2).
ARM compiler: GNU Arm Embedded 10.2.1 in DaisyToolchain.
The default DaisyExamples location is named in Makefile and dependencies.json.
`verify_dependencies.py` rejects any changed/missing input; changing a location
or dependency requires a deliberate new pin and rebuild, not silently bypassing it.
The linked libDaisy archive is prebuilt; four existing dependency-source edits
are outside this run. The archive and 180 other inputs are hashed.

## Connect and operate

Use the POD audio input L as the mono source. Input R is ignored. Both audio
outputs carry the same mono result; use either. This is codec/line-level firmware
and does not establish a guitar input-impedance or analog-front-end specification.
48 kHz, 48 frames. Startup: DIGI SINGLE, 400 ms, 0.35 feedback, 35% mix,
6000 Hz high-cut, zero drift, D2 ratio 0.75, enabled, trails on.
Wait at least 100 ms for control acquisition after audio starts.

| Control | Action |
|---|---|
| Encoder turn | DIGI -> TAPE -> MOD -> REV -> FREEZE; reverse rotation wraps |
| Encoder click | Cycle pages 1, 2, 3 |
| Page 1: K1 / K2 | TIME / FEEDBACK |
| Page 2: K1 / K2 | MIX / COLOR |
| Page 3: K1 / K2 | MOTION / D2 time ratio (ratio only in DIGI) |
| B1 | Toggle bypass |
| B2 short press/release | Tap time; in FREEZE, capture when idle or release when active |
| Hold B2 + encoder turn | DIGI: Single / Series / Parallel; FREEZE: Capture / Hold / Accumulate / Replace |
| Hold B2 + encoder click | Toggle trails |
| Hold B2 + B1 | Clear FREEZE, without toggling bypass |

Short means under 500 ms. A shifted gesture consumes the B2 release. Knobs retain
stored per-mode values until moved at least 0.012 after startup/page/mode entry;
this is an until-moved gate, not value-matching soft pickup. No preset persistence.
Tap is clamped to the selected mode's TIME range (MOD tops out at 50 ms).
D2 ratio steps: 0.5, 2/3, 0.75, 1, 4/3, 1.5, 2; resulting D2 time is 20..2000 ms.

| Mode | TIME | FEEDBACK | COLOR | MOTION |
|---|---|---|---|---|
| DIGI | 20..2000 ms | 0..0.90 | 500..12000 Hz high-cut | 0..5 ms drift |
| TAPE | Transport time | Repeats | Age | Instability |
| MOD | 0.5..50 ms base | Signed resonance | Modulation depth | LFO rate |
| REV | Slice length | Reinjection | Reverse amount | Grain taper |
| FREEZE | Capture length | Loop decay | Damping | Drift/evolution |

The last four modes retain source descriptor ranges in PedalDelayEngine.
The demonstrator requests the wet branch and applies one external mono dry/wet
blend. In MOD this deliberately replaces the reference engine's internal comb
blend with external dry/wet; the two blend laws are not claimed identical.

LED1 mode colors: DIGI green, TAPE amber, MOD blue, REV purple, FREEZE cyan.
It dims in bypass. LED2 page colors: 1 green, 2 red, 3 blue. While SHIFT is held,
DIGI configuration colors are green/amber/blue; FREEZE operation colors are
amber/blue/purple/cyan for Capture/Hold/Accumulate/Replace. Reconfiguration pause
uses red. Color mapping and controls are source-verified, not physically observed.
Trails on may keep a frozen loop audible in bypass; turn trails off for dry bypass.

## Boundaries and evidence

Mode/config changes discard tails after fading to dry for 240 samples. Main then
clears/reconfigures storage only while audio acknowledges its dry-only state;
return to wet also fades for 240 samples. Freeze Hold/Accumulate preserve capture.
TIME changes slew and can bend pitch. Final output is limited to +/-1; feedback
storage in DIGI is bounded to +/-16. These are containment choices, not clean-audio
or pop-free guarantees. D2 uses an independent mono history. The four referenced
modes still compute two duplicate mono channels internally; this is not a memory
optimization or native product-mono migration of that engine.

Executed: exact DIGI impulse/feedback oracles across ring wraps, fractional reads,
3 x 60-second feedback soaks, nonfinite input/parameters, all five audio modes,
mono output identity, pause handshake, bypass, Freeze states, storage guards,
parameter-page pickup and shifted-gesture ownership. See verification.log.

ARM link: FLASH 102832 B (78.45% of 128 KiB), SRAM 51704 B, RAM_D2 16960 B,
SDRAM 1728128 B. BOOT_NONE, internal flash. These are allocation/build evidence.

QAE raw result: 1 error, 0 warnings over the firmware files. The single
`MAKEFILE-NO-DAISYSP` finding assumes DaisySP is compulsory. It is an identified
false positive: core/Makefile guards the library with `ifdef DAISYSP_DIR`, this
adapter uses no DaisySP symbols, and the actual ARM link succeeds with `-ldaisy`.
QAE itself is not reported PASS and its raw result is preserved in qae.log.

POD 2026-09-09: seven diagnostic cells and 12,288 target impulse comparisons PASS;
14,336 DWT brackets, zero observed overruns/errors. Normal image flash/readback
verified and audio callback entry observed. See [HARDWARE_TEST.md](HARDWARE_TEST.md)
for measurement boundaries, raw evidence and backup/restore instructions.

NOT_RUN: physical knob/LED gesture observation, cold boot, extended stress,
cache/SDRAM characterization, full ISR timing, analog measurements and listening.
BBD/tape provenance/product acceptance and remote publication are still HOLD.
No Claude review was performed. The Fable-style instruction critique is in
CONTRACT.md and is a same-agent advisory, not an independent model verdict.
The DVPE file is a structural plan; editor import and visual rendering were not run.

The Makefile refuses generic `program` and `program-dfu`. The user-authorized
hardware run used an explicit UID-guarded OpenOCD command after two matching backup
reads. The normal demonstrator is loaded; repeat/recovery details are in
HARDWARE_TEST.md. The previous firmware backup is retained locally outside the ZIP.
