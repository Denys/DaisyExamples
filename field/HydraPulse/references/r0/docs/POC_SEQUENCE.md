# Staged proof-of-concept sequence

HydraPulse uses completion gates that separate software plausibility from physical truth.

## R0 - Repository foundation

**Goal:** create a deterministic, hardware-independent base that can be tested on every commit.

Implemented in this bootstrap:

- fixed-size 16-step pattern state;
- deterministic sequencer;
- sample-domain tempo clock;
- soft takeover;
- host tests and deterministic stress tests;
- repository/provenance validation;
- ASan+UBSan CI definition.

**Gate:** native build + CTest + validators pass. Sanitizer CI is defined but is not considered executed until GitHub runs it.

## P0 - Field Truth Test

P0 is a hardware characterization milestone, not a feature demo.

### P0.1 Build truth

- add `firmware/field_truth/FieldTruth.cpp` and project-local build metadata;
- pin and fetch libDaisy;
- build libDaisy;
- build Field Truth for ARM;
- preserve exact build log and firmware hash.

**Gate:** ARM build passes. This does **not** complete P0.

### P0.2 Control truth

Measure and record:

- all 16 raw keyboard scan indices versus physical keys;
- eight knob raw/min/max and direction;
- four CV input raw/min/max across known applied voltages;
- Gate In low/high behavior and edge detection;
- MIDI receive event types and raw values;
- OLED/LED basic functionality.

### P0.3 Output truth

Measure:

- CV Out 1 and 2 voltage versus requested code;
- Gate Out low/high voltage;
- output safety behavior during boot/reset.

### P0.4 Audio truth

With a documented source/load/measurement fixture:

- stereo passthrough gain;
- frequency response sufficient to expose gross path errors;
- clipping/headroom;
- noise floor under a defined bandwidth/method;
- channel crosstalk;
- cold-boot behavior.

### P0.5 Callback truth

Instrument full callback entry-to-exit time. Record:

- sample rate;
- block size;
- callback period;
- min/mean/p99/p99.9/max or the closest feasible distribution summary;
- overrun/deadline count;
- test duration and load condition;
- probe point / GPIO method if external timing is used.

**P0 completion gate:** physical evidence exists for controls, audio, CV/Gate, MIDI, and callback timing. Compilation alone cannot satisfy P0.

## P1 - Beat Core

Only after P0:

1. Add stable voice interface.
2. Implement Hammer / Crack / Steel / Arc.
3. Keep Tune / Decay / Character / Level semantics common.
4. Drive voices from the R0 sequencer/clock.
5. Add deterministic sonic scenarios on host where practical.
6. Compare host numerical behavior with target captures where practical.
7. Add audio loopback evidence on Field.

**P1 completion gate:** four voices sequence reliably on Field, deterministic regressions pass, and real target audio/timing evidence shows credible margin.

## P2 - A/B/Fill

Add direct performance variation before deeper sequencing. Preserve the 16-step surface.

## P3 - Advanced sequencing

Add only when direct operation remains coherent:

- microtiming;
- ratchets/retrigs;
- probability/conditions;
- parameter locks.

## P4 - Hydrasynth interaction

Deferred until standalone proof:

- MIDI clock/control;
- analog Pitch/Gate/Mod interaction;
- clock jitter/drift comparison;
- optional stereo audio-through;
- ducking / rhythmic destruction.
