# Test strategy

## Evidence layers

HydraPulse does not collapse unlike tests into one green badge.

| Layer | What it proves | What it does not prove |
|---|---|---|
| Host unit tests | deterministic core semantics | ARM timing, Field I/O, audio quality |
| Deterministic stress tests | repeatable invariants under many operations | true fuzz coverage or target behavior |
| ASan/UBSan | common host memory/UB failures | embedded memory map correctness |
| ARM build | toolchain/API/build integration | electrical behavior or real-time margin |
| Target instrumentation | execution on MCU | analog audio/CV quality by itself |
| Bench measurements | actual physical behavior | general correctness outside measured conditions |

## R0 native tests

### Pattern

- default clear state;
- set/toggle/clear;
- bounds behavior;
- full 16-bit mask;
- deterministic 20k-operation property stress against a reference model.

### Sequencer

- stopped state emits nothing;
- Start begins at step zero;
- 16-step wrap;
- deterministic ascending track trigger order;
- Stop halts emission;
- restart policy is explicit.

### Sample clock

- exact 120 BPM / 48 kHz / 16th-note interval;
- non-integer BPM long-run schedule;
- event location remains within one sample of ideal time;
- invalid tempo/sample-rate rejection.

### Pickup

- no premature parameter jump;
- capture inside tolerance;
- capture on crossing;
- direct tracking after capture;
- normalized clamping.

## CI policy

R0 CI runs:

1. ordinary host configure/build/CTest;
2. repository validator;
3. provenance validator;
4. Python validator tests;
5. independent ASan+UBSan build/test job.

ARM/libDaisy build enters CI with P0. No public CI workflow may flash physical hardware.

## Future P1 sonic regression

Each synthesized voice will receive deterministic scenarios such as:

- fixed trigger cadence;
- fixed parameter tuples including corners;
- fixed sample rate/block size;
- statistical and waveform-derived signatures that tolerate intended floating-point variation;
- explicit target-vs-host comparison where the same algorithm runs on both.

Waveform hashing alone is not sufficient if benign compiler/FPU differences cause false failures. Use decision-useful metrics: peak/RMS, decay time, spectral centroid/bands, zero-crossing or envelope characteristics, and bounded numerical tolerances.
