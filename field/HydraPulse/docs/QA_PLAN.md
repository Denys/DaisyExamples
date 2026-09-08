# Qualification plan: software evidence, then hardware truth

## Acceptance model

A result has an evidence class, a tested object and a scope. Use
`VERIFIED | DERIVED | PROPOSED | ASSUMED | HOLD | NOT_RUN`.
`VERIFIED` does not mean universally correct: name the exact source/build,
procedure, stimulus, duration, fixture and observed result.

The stages are independent:

| Gate | Required evidence | What it does not prove |
|---|---|---|
| R0/core | Host tests, negative tests, exact scenario replay | ARM timing or peripheral operation |
| Adapter contract | Actual firmware translation units compiled against declared fake BSP; application routing tests | Actual libDaisy ABI, UART, codec, DMA, linker |
| ARM | Exact pinned dependencies, full rebuild, ELF/BIN/MAP and hashes | Power-up, analog performance or real-time margin |
| P0 | Measured Field I/O, mapping and callback evidence with raw artifacts | Musical usability of four voices |
| Beat qualification | Target stress, audition, interface tests and timed loopback | Full seven-voice product or Hydrasynth integration |

The delivered Beat code is an implemented candidate, not a declaration that
P0 has passed. Review `VALIDATION.md` for checks actually executed.

## A. Native automation

Run the commands in `CODEX_EXECUTION.md`. The current CTest registration is the
source of truth for executable names.

### Pattern and transport

Exercise all valid and invalid track/step bounds, toggle twice identity, clear
isolation, deterministic property stress, stable ordering, 16-step wrap, stopped
silence, start at step zero, repeated Start/Stop, A/B bar-boundary switching and
momentary Fill return. Check A/B editing versus the playing bank separately.
A mute should neither delete steps nor reappear as a latent stuck trigger.

Test simultaneous bank requests, Fill edge near step zero, preset confirmation
while playing, and stopping immediately before a boundary. Presets must refuse
to load while running. Start/stop/cancel must remain meaningful while a preset
transaction is fading.

### Clock

Check 44.1/48/96 kHz and integer/fractional BPM; compare rational ideal event
positions, not wall-clock execution speed. Error against the sample-quantized
schedule is at most one sample in the tested fixed-tempo scenarios. Test
repeated identical tempo updates, changed tempo mid-interval, extreme legal
tempo, phase preservation and swing pair length. Swing must commit at a pair
boundary without shifting the pair's total duration. Check invalid rates,
NaN/Infinity, out-of-range tempo and transactional rejection.

Extend the long-run test to a representative hour or an equivalent event-domain
reference before any external clock implementation. No MIDI-clock drift claim
exists for a core that deliberately ignores external clock.

### Control gestures

Use deterministic 1 kHz stable input frames. Cover tap release, hold threshold
minus/at/plus one tick, release ordering, keys already held when Shift is
pressed, Shift released before the key, both switches released in either order,
repeated panic, switching voices with knobs elsewhere, and preset-load pickup.
Confirm no modifier gesture leaks a normal step toggle on release.

Check the exact beginner path without a menu browser:
select voice, enter steps, edit four voice controls, Play.
Confirm a step hold toggles accent once, not at every scan. Clear requires a
stopped transport and a one-second hold. Preset confirmation requires stopped
transport; preview expires after five seconds.

### Concurrency and bounded work

The audio callback owns controls, Engine and DSP. Foreground is the sole
producer of MIDI commands and the sole consumer of UI snapshots. Tests include
queue capacity, wrap and 200,000 ordered producer/consumer transfers. Capacity
is N-1, not N. Overflow must be visible and must never corrupt adjacent state.

The callback drains at most eight ordinary MIDI commands per block. Urgent
stop/panic bypasses the ordinary queue and flushes at most its fixed capacity.
The no-allocation test watches callback-equivalent engine/controller processing.
Review the real firmware callback for accidental logging, display, storage,
formatting, heap allocation or blocking calls as well. A passing heap hook
does not detect every possible third-party allocation or interrupt stall.

### DSP and sonic scenarios

Render each voice across the parameter corners and long tails. Test finite
output, normalized bounds, zero-input silence, tail convergence, frequency
range limits, parameter ramp endpoints, repeated release, rapid retrigger and
preset transitions. Each voice has an independent deterministic noise seed.

Use matched state trajectories to isolate discontinuities:
run two identical voices; apply a retrigger or parameter jump to only one; compare
the first affected sample against the uninterrupted trajectory. A large raw
adjacent-sample difference in a noisy snare is not automatically a click.
The current differential regression threshold is 0.08 full scale for the
selected retrigger cases, with explicit test fixtures and recorded maxima.
It is not a universal psychoacoustic audibility threshold.

The first candidate failed this test because Arc reset modulation depth
instantly. The repair ramps its transient envelope from current state and
smooths the noise-filter coefficient. Keep the before/after regression evidence.

`render_scenarios` renders eight deterministic ten-second stereo scenarios:
A starts, B is requested at 3 s, Fill is held from 5 to 6 s, and panic occurs
at 7 s. Check the last second is silent. Two runs on the same compiled object
must produce identical PCM and metrics. Check all samples/metrics are finite,
all eight files have the documented format, blank preset is silent, nonblank
presets have energy, final faults are zero and peak is below 0.737 normalized.
No bit identity across different architectures/compilers is promised.

Retain peak, RMS, DC mean, maximum adjacent delta, tail peak and fault counts,
but do not mistake these six metrics for a complete sound-quality test.
Before musical release add: oversampled reference comparison, alias-energy
sweeps, spectral-centroid/band-energy scenarios, pitch/decay tolerance checks,
channel balance, and target/host numerical comparison under documented
floating-point tolerance. The present Arc and saturation path is not oversampled.

### Sanitizers and build variants

Run Release, Debug+UBSan, Debug+ASan+UBSan, and a second compiler. Build both
firmware translation units under the host contract check. Verify the intentional
NaN-negative test exits for its assertion, not a sanitizer startup failure.
Host memory/threading tests do not prove interrupt or DMA/cache coherence.

## B. Repository, documentation and evidence tests

Validate generated preset freshness, original R0 hashes, full dependency-pin
format, source/provenance entries, isolation of the host core from BSP includes,
all required files and all eight Mermaid source/Markdown pairs.

Python negative tests must reject malformed/nonfinite presets, inconsistent
accent masks, duplicate/malformed telemetry, unqualified hardware evidence,
missing/hash-mismatched raw artifacts, unsafe paths, conflicting destination
files and symlink traversal. Installer conflict detection is a complete
preflight; unrelated destination content must remain byte-identical.

After integration verify actual recursive dependency gitlinks, cleanliness,
license blob and real required API files. This is deliberately separate from
offline JSON validation. Render all Mermaid diagrams with the real CLI and
review them. The manual, control map and factory data must agree with code.
Check all documented gestures by test, and all rendered display strings for
the 128x64 layout.

The inspected logger uses a 128-byte message buffer. Test conservative maximum
numeric widths, string precision and room for CRLF/NUL; split telemetry rather
than relying on silent truncation.

## C. Actual ARM qualification

Build both Truth and Beat against the pinned dependency. Save compiler version,
tool paths, full source fingerprint, parent Git SHA and dirty state, recursive
gitlink records, flags, full log, link map, section sizes and image hashes.

Verify no unintended host stub include path enters the ARM command. Confirm
default-width enums on both sides of public MIDI API boundaries. Inspect
float ABI/CPU flags, linker memory layout, static allocation and stack reserve.
Do not accept source pinning alone as bit-reproducible toolchain evidence.

For the selected actual Seed/bootloader, verify that the compiled APP_TYPE
matches the programming method. Check internal-flash bounds for BOOT_NONE;
otherwise make an explicit bootloader/layout decision. Do not increase limits
on the assumption that an undocumented memory region happens to work.

## D. P0 Field Truth bench sequence

Start with Truth, no connected CV/Gate destination, and muted/low monitoring.
Use the actual manufacturer's board documentation for electrical limits.
Hydrasynth and DFAM manual voltages are not Field specifications.

Record Field/Seed revisions and identifying photos, power configuration,
firmware SHA-256/build ID/APP_TYPE, operator, date/time, temperature when useful,
measurement instruments and settings, leads/probes/loading, audio interface
gain/sample rate, fixture wiring and calibration checks. Keep raw captures;
derived plots or a filled checklist alone are insufficient.

| Test | Stimulus and raw artifact | Completion criterion |
|---|---|---|
| BOOT | True power interruption and rail/power video or trace; serial from earliest available connection; repeat with USB absent/present | Explicit distinction between power-on, reset and late USB enumeration; deterministic stopped state and output behavior |
| KEYS | Every physical key pressed/released individually and in chords; video plus raw key log | All 16 unique raw indices and actual LED relationships mapped; no missing/duplicate stable events in the test |
| KNOBS | Slow full-range sweeps and stationary endpoints for all eight controls; serial CSV | Monotonic practical behavior, endpoints and idle spread recorded; pickup reachable despite real tolerances |
| CVIN | Four individually driven inputs with calibrated bounded source and DMM | Transfer/range/polarity/saturation derived from actual voltage, not normalized ADC numbers alone |
| GATEIN | Pulse generator and scope at defined levels, frequencies and widths | Measured levels and tested detection envelope; qualification includes the 1 kHz polling limitation |
| CVOUT | SW1 release-to-arm; DAC codes 0,1024,2048,3072,4095 on both channels; DMM with known loading | Measured voltage/code table, fitted gain/offset/residual, inactive and release behavior |
| GATEOUT | Armed/unarmed and switch-release combinations with scope | Measured low/high levels and release latency; no claim of hardware fail-safe |
| MIDI | Known messages from independent source, raw transmit log and receive log | Byte/event/channel agreement, isolated overflow test, disconnect/reconnect behavior |
| DISPLAY | Individual controls and SW2 Vegas mode, photos/video | All required LEDs and OLED pixels/status fields exercised; map recorded |
| AUDIO | Calibrated interface loopback then Field loopback, stimuli and raw stereo WAVs | Gain, noise, headroom/clipping, crosstalk and routing characterized relative to baseline |
| TIMING | Scope/logic analyzer on qualified timing testpoint, telemetry, stressed UI/MIDI | Callback interval, maxima and observed deadline margin measured with fixture details |

The firmware's Seed testpoint use is a **proposed probe route**, not proof that
it is conveniently accessible on an assembled Field. Inspect the actual board
and continuity first. Do not attach a probe to a presumed unused signal based
on a package name. If unavailable, choose an electrically safe verified
alternative and record the code/fixture change.

`SetCvOut*(0)` is a commanded minimum DAC code, not a measured zero-volt
guarantee. The momentary output interlock requires an initial release of SW1,
then SW1 held alone; SW2 inhibits output testing. It is software logic and
cannot remove voltage after a CPU crash. Use an appropriate external test
fixture and disconnected load.

The gate input is polled once per 1 ms callback. Pulses narrower than that can
be missed; characterize it rather than describing the diagnostic as a
sample-accurate external clock receiver. Future clock input needs a separately
qualified capture/timestamp design.

### Proposed engineering thresholds, to ratify against the fixture

These are project acceptance targets, **not measured results or manufacturer
specifications**:

- Zero unexpected resets, fault counts, telemetry/key losses, or unexplained
  I/O changes during the normal diagnostic session.
- Maximum instrumented callback body below 0.5 ms at 48 samples/48 kHz under
  worst exercised UI and MIDI activity; zero observed >1 ms body intervals and
  zero reported >1.5 ms start gaps. Measure full IRQ/DMA overhead separately:
  the callback probe does not include work outside the application callback.
- Knob endpoint residuals/pickup tolerance ratified from measured spread; do not
  hardcode an arbitrary claim of exact 0.000/1.000 physical endpoints.
- CV output monotonicity and residual within 1% of the **measured** span after
  affine fitting, unless the hardware requirement calls for tighter calibration.
  This target is not sufficient for claiming precision 1 V/oct tuning.
- Audio round-trip gain repeatability within 0.2 dB under an unchanged fixture,
  no unexplained channel reversal, and clipping/noise/crosstalk boundaries
  explicitly measured. Choose absolute performance limits only after measuring
  the direct-interface baseline and establishing required source/load levels.

Do not convert an untested item into PASS to reach an aggregate milestone.
Use `hardware/p0_record.template.json`, add real metadata and hashed raw files,
then run `tools/validate_evidence.py`. That validates records, not their truth.

## E. Beat Core on-hardware qualification

After credible P0: flash the separately hashed Beat build; verify all controls
from the manual, voice identification, master pickup and stopped boot. Check
initial master .25 does not become a high physical-knob value without pickup.

Run every preset and full A/B/Fill scenario. Compare recorded target output
with deterministic host renders after removing measured latency and fixture
gain; define numerical/spectral tolerances before judging. Listen on conservative
monitoring levels for zipper noise, retrigger discontinuities, truncation,
stereo imbalance, persistent tails, aliasing and unintended ringing. Intended
percussive attacks and saturation must not be mislabeled as arbitrary faults.

Exercise rapid accents, long decays, all voices, high drive, parameter sweeps,
mute/unmute, multiple gestures, preset loads, repeated Panic, MIDI bursts,
USB disconnected/connected and prolonged idle. Suggested initial soak is
60 minutes with full raw error/callback summaries and representative scope/audio
captures; longer reliability work remains a product gate.

For every audible defect preserve: build ID, preset, knob values, gesture/MIDI
sequence, time range, raw audio and shortest deterministic host reproducer.
Do not tune a limiter to hide the fault without fixing its cause.

## F. Deferred qualification, not hidden promises

No current claims for persistence/power-loss recovery of saved patterns,
seven voices, sampling, probability/ratchets/microtiming/locks, MIDI clock sync,
CV musical modulation, audio-through, Hydra-specific integration, or a valid
DVPE graph. Add each as a separate testable requirement after the independent
instrument proves useful.

This plan and its accompanying test code are not an independent audit. Formal
musical release still needs actual hardware evidence and a separate review
appropriate to the intended distribution.
