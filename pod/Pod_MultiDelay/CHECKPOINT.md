# Pod MultiDelay v0.5 - 2026-11-14

Primary lane: Delay. Source: `pod/Pod_MultiDelay`, branch
`claude/pod-multidelay-v0-5-drive-oled`, based on v0.4 @ f0fe2863.

Status: `HOST_TEST_PASS / ARM_BUILD_PASS / FLASHED_2026-11-14 / SYSTEM_CHECK_PASS /
PHYSICAL_CONTROLS_NOT_RUN / LISTENING_NOT_RUN / OLED_VISUAL_NOT_CONFIRMED`.

Changes from v0.4:
- DRIVE replaces DIST: OVERDRIVE / DISTORTION / FUZZ, first-order ADAA clipping,
  GAIN up to +30 / +50 / +66 dB, TONE, LEVEL, BLEND, FUZZ gate, per-type settings,
  type change through a 5 ms mute (`Drive.h`).
- DRIVE is entered and left with a B1+B2 chord (80 ms window) from any delay page
  and returns to the previous page; inside, turn and SHIFT+turn select the type,
  click switches GAIN/TONE and LEVEL/BLEND, B1 toggles DRIVE (`Controls.h`).
- B1 alone now acts after the chord window (or on an earlier release).
- OLED visual navigation on I2C1 D11/D12 at 400 kHz, one bounded transaction per
  1 ms tick, dirty pages only, hot-plug retry (`Ui.h`, `Pod_MultiDelay.cpp`).
- On-target health record `g_health` (magic "POD5", version 0x500).

Validation executed in this run:
- `verify.ps1`: dependencies 181, DIGI 63, parity 1,440,000 samples bit-exact,
  demo 20, controls 43, drive 21, UI 19: PASS.
- ARM BOOT_NONE: FLASH 114,440 B (87.31 % of 128 KiB), SRAM 54,792 B,
  RAM_D2 16,960 B, SDRAM 1,728,128 B. BIN sha256 cc3bb7d1...72a3, rebuilt
  identically after clang-format 10.
- Flash (probe 0020000A5553500920393256, UID guard): two equal backups of the
  previous image (= v0.4 BIN), `Verified OK`, readback equals the BIN, CFSR/HFSR 0.
- System check over SWD after 3 s of running: OLED ACK 1, 0 I2C errors, worst
  main-loop gap 1001 us, worst I2C transaction 817 us, audio worst 38,495 cycles
  with DRIVE off; with DRIVE on (LEVEL -24 dB, set over SWD) worst 56,560 cycles
  (OVERDRIVE), 55,368 (DISTORTION, includes the type switch), 40,694 (FUZZ). At
  the libDaisy default 400 MHz a 48-sample block has 400,000 cycles: 14 % worst.

Not run: what the screen shows on the real panel, button chord feel and the 80 ms
window on hardware, listening (gain staging, gate behaviour, aliasing by ear).
Evidence: custom-pedals `delay/evidence/2026-11-14_pod_multidelay_v0_5_flash/` and
`delay/runs/2026-11-14_pod_multidelay_v0_5_drive_oled.md`.
Reproduce from this project directory: `./verify.ps1`.
