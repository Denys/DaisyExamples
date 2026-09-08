# Architecture

## Design rule

HydraPulse keeps musical timing and pattern semantics independent of libDaisy wherever practical. Hardware code adapts real Field I/O into stable core-facing values; it does not own sequencer semantics.

```text
+--------------------------------------------------------------+
| UI / performance semantics                                   |
| select voice | 16 steps | Tune Decay Character Level | Fill |
+-----------------------------+--------------------------------+
                              |
+-----------------------------v--------------------------------+
| Pure C++ core                                                |
| Pattern16 | StepSequencer | SampleAccurateStepClock | Pickup |
+-----------------------------+--------------------------------+
                              |
+-----------------------------v--------------------------------+
| Voice layer (P1)                                             |
| Hammer | Crack | Steel | Arc                                 |
| stable HydraPulse voice interfaces; DaisySP may sit behind it|
+-----------------------------+--------------------------------+
                              |
+-----------------------------v--------------------------------+
| Field adapter (P0)                                           |
| keys knobs CV gate MIDI OLED LEDs audio callback             |
+-----------------------------+--------------------------------+
                              |
+-----------------------------v--------------------------------+
| libDaisy / Daisy Field hardware                              |
+--------------------------------------------------------------+
```

## R0 core

### `Pattern16<Tracks>`

Fixed-size, allocation-free 16-step bit patterns. Bounds failures are explicit boolean results rather than exceptions. The structure is intentionally boring because musical pattern state should not depend on heap behavior.

### `StepSequencer<Tracks>`

Deterministic transport-to-pattern bridge. `Start()` restarts at step zero. Each `EmitAndAdvance()` returns a fixed-size tick containing the step index and track bitmask. Trigger enumeration is always ascending track order.

### `SampleAccurateStepClock`

Integer phase accumulator. BPM is quantized to one micro-BPM and accumulated in sample domain. This avoids cumulative floating-point drift and naturally alternates adjacent integer sample intervals when the exact interval is fractional.

The clock only defines boundaries. It does not decide whether Start should emit step zero immediately; that policy belongs to the transport/sequencer integration.

### `ControlPickup`

Normalized `[0,1]` soft takeover. A physical control remains disconnected until it reaches or crosses the stored target, then tracks directly.

## P0 Field adapter contract

The P0 adapter must prove, not assume:

- physical order of all 16 key scan indices;
- eight knob endpoints and monotonic direction;
- four CV input usable ranges;
- Gate In polarity/threshold behavior at the assembled Field;
- Gate Out electrical levels;
- both CV output transfer functions;
- MIDI receive path;
- stereo audio path and real gain/headroom/noise/crosstalk;
- OLED and LED behavior;
- callback period, execution distribution, worst observed execution, and margin.

The official `DaisyField` API inspected for R0 exposes `KNOB_1..8`, `CV_1..4`, keyboard state/edge methods, `gate_in`, `gate_out`, `midi`, OLED display, LED driver, and `SetCvOut1/2()`.

## Real-time boundary

The audio callback may perform bounded DSP, state snapshots, and constant-time instrumentation. It must not perform serial logging, OLED writes, storage access, dynamic allocation, or unbounded work.

Human-readable telemetry is emitted outside the callback from data captured by bounded instrumentation.

## Future portability

If the concept proves worthwhile, the pure core and stable voice interfaces become the portable layer. The first product should not pay the abstraction tax of a hypothetical future hardware platform before the Field instrument itself is useful.
