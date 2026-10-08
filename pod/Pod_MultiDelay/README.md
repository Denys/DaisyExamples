# POD MultiDelay demonstrator v0.4

v0.4 (2026-10-08): independent PRE distortion. Click the encoder three times
from page 1 to open DIST (yellow LED2). K1 = Drive, K2 = Tone; B1 toggles ON/OFF.
Entering/leaving DIST, pot edits and mode selection preserve ON/OFF. B1 on
delay pages still bypasses only the delay. Startup: OFF, Drive 40%, Tone 50%.
See CONTROLS.md and USER_MANUAL.md; current validation is in CHECKPOINT.md.
This is a Pod feature, not a sixth delay engine or a DIGI-topology change.

The v0.1 packager/manifest and v0.3 diagrams are historical; neither describes
the new distortion stage. Build v0.4 with `verify.ps1`; the DVPE contains its plan.

Status (v0.3, 2026-10-07): **DIGI_CONTRACT_V1_HOST_PASS / DAFX_NODE_PARITY_BIT_EXACT / ARM_BUILD_PASS / POD_FLASH_READBACK_VERIFIED / LISTENING_NOT_RUN**.
v0.3 changes only DIGI, to the accepted DIGI contract v1; see CONTRACT.md. The v0.1
status below (2026-09-09) still describes TAPE, MOD, REV, FREEZE and the shell.

v0.1 status: **HOST_TEST_PASS / ARM_BUILD_PASS / POD_DIGITAL_SCREEN_PASS / NORMAL_IMAGE_FLASH_VERIFIED**.
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
| Encoder click | Cycle pages 1, 2, 3, DIST |
| Page 1: K1 / K2 | TIME / FEEDBACK |
| Page 2: K1 / K2 | MIX / COLOR |
| Page 3: K1 / K2 | MOTION / unused (MOTION has no effect in DIGI) |
| Hold B2 + K1 / K2 (DIGI) | D2 ratio / D2 feedback (unlinks) |
| Hold B2 + B1 (DIGI) | Relink D2 feedback |
| DIST: K1 / K2 | Drive / Tone; editing never enables distortion |
| B1 on DIST | Toggle distortion ON/OFF |
| B1 on delay pages | Toggle delay bypass; preserve distortion ON/OFF |
| B2 short press/release | Tap time; in FREEZE, capture when idle or release when active |
| Hold B2 + encoder turn | DIGI: Single / Series / Parallel (no pause, tails kept); FREEZE: Capture / Hold / Accumulate / Replace |
| Hold B2 + encoder click | Toggle trails |
| Hold B2 + B1 | Clear FREEZE, without toggling bypass |

Short means under 500 ms. A shifted gesture consumes the B2 release. Knobs retain
stored per-mode values until moved at least 0.012 after startup/page/mode entry;
this is an until-moved gate, not value-matching soft pickup. No preset persistence.
Tap is clamped to the selected mode's TIME range (MOD tops out at 50 ms).
D2 ratio steps: 1/4, 1/3, 3/8, 1/2, 2/3, 3/4, 1 (D2 = round(r x D1)).

| Mode | TIME | FEEDBACK | COLOR | MOTION |
|---|---|---|---|---|
| DIGI | 20..2500 ms | k 0..0.95 | high-cut off..2 kHz | unassigned |
| TAPE | Transport time | Repeats | Age | Instability |
| MOD | 0.5..50 ms base | Signed resonance | Modulation depth | LFO rate |
| REV | Slice length | Reinjection | Reverse amount | Grain taper |
| FREEZE | Capture length | Loop decay | Damping | Drift/evolution |

The last four modes retain source descriptor ranges in PedalDelayEngine.
The demonstrator requests the wet branch and applies one external mono dry/wet
blend. In MOD this deliberately replaces the reference engine's internal comb
blend with external dry/wet; the two blend laws are not claimed identical.

LED1 mode colors: DIGI green, TAPE amber, MOD blue, REV purple, FREEZE cyan.
It dims in delay bypass and pulses when distortion is ON. LED2 page colors:
1 green, 2 red, 3 blue, DIST yellow (dim OFF, bright ON). On DIST the yellow
indicator keeps priority. On other pages while SHIFT is held,
DIGI configuration colors are green/amber/blue; FREEZE operation colors are
amber/blue/purple/cyan for Capture/Hold/Accumulate/Replace. Reconfiguration pause
uses red. Color mapping and controls are source-verified, not physically observed.
Trails on may keep a frozen loop audible in bypass; turn trails off for dry bypass.

## Boundaries and evidence

Mode changes discard tails after fading to the PRE path for 240 samples. DIGI
config changes keep tails without a pause. Main then
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
