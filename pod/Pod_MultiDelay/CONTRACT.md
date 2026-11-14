# POD five-mode demonstrator contract

Primary lane: Delay. Version 0.3, 2026-10-07 (DIGI contract v1; v0.1 2026-09-09). Local integration candidate;
product DSP authority remains DAFX. User selected DIGI/TAPE/MOD/REV/FREEZE.
POD is explicitly selected for this demonstration under ADR-0018; Field remains
the default development target. ADR-0019 supersedes the historical stereo router.

## Audio and topology (PROPOSED demonstrator laws)

48 kHz, 48 frames, mono input from codec channel L, identical mono outputs L/R.
R input is ignored. No input summing, no stereo or Ping-Pong claim.
TAPE/MOD/REV/FREEZE use the existing DaisyExamples PedalDelayEngine through
a reference to its source files, without copying its implementation.
DIGI (v0.3) implements the mono-first DIGI contract v1 exactly: custom-pedals
`delay/runs/2026-09-24_mono_first_digi_contract_v1.md`, accepted by Denys 2026-09-25.
Per engine (DAFX DigitalDelayNode @ 73976da, mono): tap = h[D] read before write,
integer D rounded half up; cond = LP(HP(tap)); h <- in + f*cond.
SINGLE: in2 = 0 (E2 decays), wet = tap1. SERIES: in2 = tap1, wet = tap1 + tap2.
PARALLEL: in2 = x, wet = 0.5*(tap1 + tap2). Output y = x + MIX*wet (Dry = 1).
Controls: TIME 20 ms..2.5 s log; SHIFT+K1 ratio 1/4,1/3,3/8,1/2,2/3,3/4,1 with
D2 = round(r*D1); k = 0.95*FEEDBACK, linked, SHIFT+K2 sets E2 alone, SHIFT+B1 relinks;
SERIES uses f = 1 - sqrt(1 - k), SINGLE/PARALLEL f = k; COLOR = feedback LP from off
(0.49 fs) to 2 kHz log; HP fixed 40 Hz; MOTION unassigned in DIGI. A config change
is applied at the next block, never clears histories, and is a hard switch (contract
7 accepted click cost). Pod adaptation (PROPOSED): a 0.004 deadband on the TIME pot,
so ADC noise does not step the integer delay. Startup DIGI SINGLE, 400 ms, k 0.35,
mix 0.35, high-cut 6000 Hz, ratio 3/4, E2 linked. Output limiting to +/-1 remains.
Evidence: test_digi (contract 10 markers, section 6 build-up, soaks), test_parity
(bit-exact against the pinned DAFX node), test_controls, test_demo.

## Runtime ownership

Audio consumes one coherent control snapshot per block. Main scans controls and
publishes a short snapshot with interrupts masked only for that copy.
Audio fades to dry over 240 samples before acknowledging a requested mode
change or destructive Freeze operation (a DIGI config change needs no pause). While paused, audio only passes
sanitized dry input; main may then reset storage/reconfigure and release audio.
Audio fades back over 240 samples. No buffer clear, allocation, peripheral I/O,
logging, spin wait or blocking synchronization is allowed in the audio callback.
Mode changes deliberately discard tails; DIGI config changes keep them. Freeze Hold/Accumulate preserve
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
