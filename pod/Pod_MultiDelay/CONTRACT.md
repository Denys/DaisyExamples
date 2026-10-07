# POD five-mode demonstrator contract

Primary lane: Delay. Version 0.1, 2026-09-09. Local integration candidate;
product DSP authority remains DAFX. User selected DIGI/TAPE/MOD/REV/FREEZE.
POD is explicitly selected for this demonstration under ADR-0018; Field remains
the default development target. ADR-0019 supersedes the historical stereo router.

## Audio and topology (PROPOSED demonstrator laws)

48 kHz, 48 frames, mono input from codec channel L, identical mono outputs L/R.
R input is ignored. No input summing, no stereo or Ping-Pong claim.
TAPE/MOD/REV/FREEZE use the existing DaisyExamples PedalDelayEngine through
a reference to its source files, without copying its implementation.
DIGI uses two original mono circular histories with fractional linear reads.

Let d1[n] and d2[n] be delayed history reads, g in [0,0.90].
SINGLE: w1=x+g*LP1(d1); wet=d1; D2 idle.
SERIES: w1=x+g*LP1(d1); w2=d1+g*LP2(d2); wet=d2.
PARALLEL: w1=x+g*LP1(d1); w2=x+g*LP2(d2); wet=(d1+d2)/2.
The two self-feedback loops are independent; SERIES is a cascade, not the
historical round-trip stereo feedback matrix. Each linear low-pass has gain
at most one, so g<1 is the fixed-delay loop stability condition. Output limiting
is an emergency bound, not proof of clean audio or nonlinear loop stability.
D2 time = clamp(D1 time * ratio, 20, 2000) ms; ratio 0.5/0.667/0.75/1/1.333/1.5/2.
TIME is slewed (intentional pitch change); COLOR controls feedback high-cut;
MOTION adds up to 5 ms slow read modulation. Startup DIGI SINGLE, 400 ms,
feedback 0.35, mix 0.35, color 6000 Hz, motion zero; D2 ratio 0.75.

## Runtime ownership

Audio consumes one coherent control snapshot per block. Main scans controls and
publishes a short snapshot with interrupts masked only for that copy.
Audio fades to dry over 240 samples before acknowledging a requested mode,
configuration or destructive Freeze operation. While paused, audio only passes
sanitized dry input; main may then reset storage/reconfigure and release audio.
Audio fades back over 240 samples. No buffer clear, allocation, peripheral I/O,
logging, spin wait or blocking synchronization is allowed in the audio callback.
Mode/config changes deliberately discard tails. Freeze Hold/Accumulate preserve
the captured loop. No pop-free or target timing claim follows from this policy.

## Controls

Encoder turn: mode. Encoder click: page (TIME/FEEDBACK, MIX/COLOR, MOTION/RATIO).
K1/K2: selected page, with 0.012 movement threshold after entry; values remain
stored per mode until a knob moves. Ratio is DIGI-only.
B1: bypass. B2 short release: tap tempo outside FREEZE; capture/release in FREEZE.
Hold B2 as SHIFT: encoder turn cycles DIGI configuration or FREEZE operation
(Capture/Hold/Accumulate/Replace); encoder click toggles trails; B1 clears FREEZE.
Using a shifted gesture consumes the B2 release so it cannot also tap/capture.
LED1: mode color; dim during bypass. LED2: page color normally, configuration or
Freeze state while SHIFT is held; red during the brief reconfiguration pause.
LED color is a reference map, pending physical observation.

## Acceptance

Host: exact integer impulse arrivals for all three DIGI configurations, independent
feedback marker oracle, 60-second feedback soak, NaN/Inf, all five modes, mono
output identity, Freeze capture/hold/clear, page pickup and gesture ownership,
pause/reset/fade behavior. ARM: actual ELF/BIN/MAP for DaisyPod, BOOT_NONE;
record toolchain, source/dependency identities and memory usage. Physical flash,
boot, controls, DWT/cache/audio and listening require a separate device run.

Hardware follow-up: the separately authorized POD run is recorded in
HARDWARE_TEST.md. Its digital screen and normal-image callback entry supersede
preparation-only hardware status. The critique below is the frozen preparation
instruction analysis; its NO_FLASH boundary describes that earlier phase.

## Instruction critique (fable-instruction-critique, E2/E3)

Verdict: a board-name-only port would be unsafe to review: the older Field adapter
allows main-thread DSP mutation while its callback runs and its docs describe
stale stereo authority. The executable handoff must bind both ownership and evidence.

Deployment facts: Windows PowerShell; dirty hub continuity files; external
DaisyExamples source; POD has two pots/two buttons/one encoder/two RGB LEDs;
PedalDelayEngine has buffer-clearing Freeze operations; local libDaisy is modified;
BOOT_NONE differs from historical BOOT_SRAM; host tests cannot establish Cortex-M7 cost.

1. Main calls SetMode/FreezeClear during Process: filter state or SDRAM is changed
   mid-callback. Fix: fade/paused acknowledgement before destructive main work.
2. Copying the old DualDelayRouter restores four histories and Ping-Pong. Fix:
   explicit two-mono-history laws above and independent marker tests.
3. B2 used for SHIFT and tap triggers an unwanted tap after a configuration change.
   Fix: consume the release after any shifted gesture and test it.
4. A successful cross-build is called ready on hardware. Fix: separate build,
   physical identity, flash, timing and listening records; no automatic flash target.
5. Dirty state append collides with other work. Fix: new bounded run record with
   source hashes, all touched paths, exact verification and finishing commands.

Rewritten operator instruction: Build the new POD adapter against the recorded
external source and library hashes. Run the host behavioral tests, then build
BOOT_NONE and inspect ELF/BIN/MAP. Reconfigure DSP only after audio acknowledges
dry-only pause. Publish coherent controls and consume shifted releases. Keep
mono Single/Series/Parallel separate from historical stereo evidence. Register
all results and paths in the run record; preserve existing dirty files. Do not
flash from preparation. Claim only checks actually executed; explicitly retain
target audio/timing/listening NOT_RUN. Re-read outputs and run the hub closing check.

This shape binds each operator action to an observable result and failure boundary.
