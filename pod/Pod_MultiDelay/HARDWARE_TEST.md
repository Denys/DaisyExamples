# POD hardware functional screen — 2026-09-09

Status: **DIGITAL_SCREEN_PASS / NORMAL_IMAGE_FLASH_VERIFIED / AUDIO_CALLBACK_REACHED**.
Primary lane: Delay. Physical audio and control acceptance: **NOT_RUN**.

The user requested this exact project location, authorized useful physical tests,
and confirmed the ST-LINK/SWD cable is connected to the intended POD. This run
supersedes the preparation-only hardware status; it does not change the DSP source.
The board is currently running the normal demonstrator, not the muted test image.

## Executed evidence

- Target: user-confirmed POD; SWD-identified Cortex-M7, STM32H750, 128 KiB internal
  flash. UID 0x00300035 / 0x33335117 / 0x33353634. STLINK-V3 serial
  0020000A5553500920393256, firmware V3J7M2; observed probe voltage 3.19..3.25 V.
  This is probe telemetry, not a calibrated rail measurement. USB COM3 is ST-LINK
  VCP; no Daisy CDC logger was available. Target identity differs from the old Field.
- Before programming, two complete 128 KiB internal-flash reads matched byte for
  byte. No QSPI, option bytes, power switching or rewiring was performed.
- Diagnostic BOOT_NONE image programmed and verified through SWD. Physical DAC
  outputs were always zero. Stimulus existed only inside the diagnostic callback.
- 12,288 exact zero-feedback DIGI impulse comparisons passed on the target:
  Single at 101 samples, Series at 250, Parallel half-amplitude at 101 and 149.
- All seven live audio DMA cells completed: 128 warmup blocks and 2,048 measured
  blocks each; 48 frames at 48 kHz. Sine stimulus amplitude 0.05 for the first
  1,024 measured blocks, then silence. Default parameters, wet mix 1; FREEZE captures.
- Zero observed nonfinite samples, mono-output mismatches, buffer sentinel errors,
  aborts, and bracket deadline violations. All modes produced digital output and
  samples during the silent-input portion; FREEZE ended in Hold. This does not
  identify subjective sound quality or exhaustively establish algorithm correctness.
- Main/control-scan heartbeat and audio callbacks advanced between two captures,
  across 98502 ms. Button/encoder event counts were zero. ADC values were read;
  pot sweep, gesture ownership and LED mapping were not physically exercised.
- Host tests and normal ARM build were rerun in this destination directory: PASS.
  8,713,737 DIGI sample/guard checks, 17 demo checks and 9 control checks. Source and
  prebuilt library dependency hashes matched the pinned manifest.
- Normal image then programmed with OpenOCD verification. A separate readback of
  all 102,832 image bytes matches the built BIN. A hardware breakpoint observed
  normal AudioCallback entry at 0x08001320 in DMA interrupt context. CFSR/HFSR
  were both zero before that check. Breakpoint removed and CPU resumed on exit.
  This is callback entry evidence, not end-to-end analog acceptance or cold boot.

## DWT callback-bracket measurements

CPU 400 MHz; one block deadline is 400,000 cycles / 1 ms. The bracket includes
stimulus synthesis, Demo::Process, digital sample checks and zeroing both DAC
outputs. It excludes timing-record storage, exception entry/exit and the enclosing
ISR wrapper. Values are percentages of one block deadline, not whole-system CPU
utilization. They describe this short diagnostic workload only. Quantiles are
observed nearest-rank values, not worst-case guarantees.

| Cell | Measured blocks | Mean | p99.9 | Maximum | Overruns |
|---|---:|---:|---:|---:|---:|
| DIGI Single | 2048 | 6.75% | 7.65% | 7.69% | 0 |
| DIGI Series | 2048 | 8.57% | 9.58% | 9.61% | 0 |
| DIGI Parallel | 2048 | 8.62% | 9.59% | 9.69% | 0 |
| TAPE | 2048 | 34.07% | 35.70% | 35.84% | 0 |
| MOD | 2048 | 13.89% | 15.07% | 15.13% | 0 |
| REV | 2048 | 22.15% | 24.63% | 24.68% | 0 |
| FREEZE | 2048 | 29.88% | 34.05% | 34.19% | 0 |

The 14,336 uint32 timing samples are retained in timings.bin (sorted per cell by
firmware). Host recomputation verified every mean, p99.9, maximum and overrun count.
No sustained target stress, maximum-feedback sweep, parameter sweep, cache/SDRAM
characterization, external GPIO timing, cold power cycle or calibrated audio run
was performed. The bundled skill planner supports an advanced Field campaign;
its planner/inventory files are preserved as pre-test capability records and do
not establish or negate this project-specific POD digital screen.

## Files and reproduction

Raw records, logs, backup, parser and checks: hardware_evidence/20260909.
Diagnostic source plus the exact flashed diagnostic ELF/BIN: diagnostic/.
The normal source remains in the project root. Linked libraries remain external.
The ZIP includes diagnostic source and selected evidence; the unknown previous
firmware backup stays in the local hardware_evidence folder and is excluded from
the ZIP. No remote publication was requested or performed.

```powershell
# Read-only verification of saved evidence; no hardware connection.
py -3 -B .\hardware_evidence\20260909\verify_evidence.py
.\verify.ps1
py -3 -B .\package.py
# Optional diagnostic rebuild only, no programming:
& 'C:\Program Files\DaisyToolchain\bin\make.exe' -C diagnostic -j2
```

The initial normal boot inspection used an unsupported lowercase xpsr register
name in OpenOCD and exited after halting. normal_boot.log preserves that failed
inspection. The corrected normal_boot_verified.log records the callback hit,
breakpoint removal and resume; it is the final boot evidence.

## Recovery and repeat programming

Normal BIN SHA-256: 17924fb0991b2e572dc87027d9b925bda07ba9e92fcfe49e390c48087d435a03.
Diagnostic BIN SHA-256: f2dc2a18a3b0b368b003942ca8f7357cc3b4edad78f0869cfd067bc23b470121.
Previous 128 KiB backup SHA-256: 9545c0db36c79195d170f452830674b979a7f8a9d60f86511037b25cacf553af.

The following explicit command restores only the saved internal flash. It is
documented for recovery and was **not executed**, because the requested normal
demonstrator is the final image. Run from this project directory on the same POD.
It verifies probe serial, target UID and backup hash before programming; stop if
the connected device differs. Do not use mass erase or an unbound program command.

```powershell
$podBackup = (Resolve-Path -LiteralPath '.\hardware_evidence\20260909\original_internal_flash.bin').Path
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $podBackup).Hash -ne '9545C0DB36C79195D170F452830674B979A7F8A9D60F86511037B25CACF553AF') { throw 'Backup hash mismatch' }
$podOcd = 'C:\Program Files\DaisyToolchain\bin\openocd.exe'
$podGuard = 'set u [read_memory 0x1FF1E800 32 3]; if {[lindex $u 0] != 0x00300035 || [lindex $u 1] != 0x33335117 || [lindex $u 2] != 0x33353634} {error "Wrong target UID"}'
& $podOcd -f interface/stlink.cfg -f target/stm32h7x.cfg -c 'adapter serial 0020000A5553500920393256' -c 'adapter speed 1000' -c init -c $podGuard -c ('program {' + $podBackup.Replace('\','/') + '} 0x08000000 verify reset exit')
if ($LASTEXITCODE -ne 0) { throw 'Restore failed; inspect OpenOCD output before further action' }
```

The application Makefile still refuses generic program/program-dfu. This authorized
hardware run used an explicit identity-guarded OpenOCD BOOT_NONE command. For a
future normal-image load, use the same guard and replace the final program command
with the absolute build/Pod_MultiDelay.elf path followed by verify reset exit.
Re-check the artifact and hardware setup before a future physical run.

Remaining manual acceptance: connect a known line-level mono source to input L,
listen at a low monitor level, confirm all five modes, bypass/trails, tap, three
parameter pages, shifted gestures and FREEZE capture/release. Current host tests
cover the corresponding state-machine logic; physical controls and audio remain
unverified. No independent Claude review or product-source promotion is claimed.
