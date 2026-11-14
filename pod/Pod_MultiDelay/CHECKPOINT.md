# Pod MultiDelay v0.4 - 2026-10-08

Primary lane: Delay. Source: `pod/Pod_MultiDelay`, branch
`codex/pod-multidelay-distortion`, based on the locally verified v0.3 candidate.

Status: `HOST_TEST_PASS / ARM_BUILD_PASS / NO_FLASH_STLINK_OPEN_FAILED /
PHYSICAL_CONTROLS_NOT_RUN / LISTENING_NOT_RUN / TARGET_TIMING_NOT_RUN`.

DIST is page 4, selected by encoder click. K1 Drive / K2 Tone. B1 on DIST
toggles distortion; B1 on delay pages toggles delay bypass. Encoder navigation,
parameter editing and delay-mode selection preserve distortion ON/OFF. This
keeps the five delay modes and DIGI contract v1. PRE distortion also processes
the dry branch and continues through delay bypass and mode-reconfiguration pause.
Defaults: OFF / Drive 40% / Tone 50%. All settings are session-only.

Validation executed in this run:
- Original v0.3 baseline: dependencies 181, DIGI 63, parity 1,440,000 samples,
  demo 20 and controls 19: PASS.
- v0.4: controls 40, distortion 12, demo 20, DIGI 63: PASS. Parity remains
  bit-exact across 1,440,000 samples with the DAFX node at the existing pin.
- ARM BOOT_NONE: FLASH 104,508 B (79.73%), SRAM 51,960 B, RAM_D2 16,960 B,
  SDRAM 1,728,128 B. No dependency/library/submodule changes.
- QAE: 1 error / 0 warnings, the pre-existing MAKEFILE-NO-DAISYSP false positive;
  actual build links with libDaisy only. QAE is not reported PASS.
- Probe command bound serial 0020000A5553500920393256, with target UID guard:
  OpenOCD returned `Error: open failed` before target access. No flash, erase,
  backup or target halt was performed. Prior flash records are historical.

Drive uses bounded cubic clipping and a smoothed tone low-pass at 48 kHz,
without oversampling. Enable ramp is 240 samples. No fixed added latency when
OFF. Tests establish finite/bounded output, clean identity when OFF, independent
Drive/Tone response, pre-delay routing and continued operation in delay pause.
Aliasing, physically pop-free switching, MCU CPU margin and listening are NOT_RUN.

CI publication repair: repository clang-format 10 applied to all eleven PR C++
files; normal and diagnostic Makefiles now default to repository-relative paths
(verify.ps1 still passes the pinned local source root explicitly). The muted
diagnostic's obsolete DIGI call and Series marker were aligned with contract v1.
Diagnostic build is checked separately; no new diagnostic target run is claimed.
Initial CI failures were formatting and a Windows-only source-root default.

Reproduce from this project directory: `./verify.ps1`.
Next physical action: reconnect the same Pod/ST-LINK, verify probe/UID and two
matching 128 KiB backups, then use the documented guarded BOOT_NONE image route
in HARDWARE_TEST.md. Confirm page entry/exit, B1 toggling, LED indication, drive
audio and delay bypass separately. The existing v0.1 ZIP/manifest/packager and
v0.3 block-diagram exports remain historical; the updated DVPE is the v0.4 plan.
