# HydraPulse Field

HydraPulse Field is a staged, standalone drum/performance instrument for the stock Electro-Smith Daisy Field.

The project is deliberately proving the instrument in layers instead of implementing the whole product at once. The first musical target is an independent Field drum machine. Hydrasynth-specific MIDI/CV/audio interaction is deferred until the standalone foundation is credible.

## Scope

### Product direction

- Stock Daisy Field is the first hardware target.
- Normal interaction stays centered on 16 visible steps.
- Beginner invariant: **select drum -> tap 16 steps -> edit sound -> Play**.
- Common voice language: **Tune | Decay | Character | Level**.
- First four synthesized voices: **Hammer** (kick), **Crack** (snare), **Steel** (closed hat / metallic), **Arc** (FM percussion).
- A/B/Fill precedes deep trig-lock, probability, ratchet, and microtiming systems.
- Hydrasynth integration is intentionally deferred.

### What this bootstrap implements

This repository bootstrap is the pre-hardware software foundation, called **R0** so it is not confused with P0 hardware truth.

Implemented now:

- hardware-independent C++17 core;
- 16-step pattern container;
- deterministic step sequencer;
- sample-domain integer timing clock;
- soft-takeover / pickup primitive;
- native CMake + CTest harness;
- deterministic stress and long-run timing tests;
- repository/provenance validators;
- GitHub CI for native tests and ASan+UBSan;
- pinned upstream provenance records;
- P0 Field Truth implementation contract and evidence layout.

Not implemented yet:

- Daisy Field firmware binary;
- ARM cross-build verification;
- any physical Daisy Field measurement;
- audio/CV/Gate/MIDI hardware evidence;
- P1 drum synthesis;
- A/B/Fill;
- advanced sequencing;
- Hydrasynth integration.

A compile is not hardware evidence, and a host test is not ARM timing evidence. This repository treats those as different claims on purpose.

## Repository layout

```text
.
├── CMakeLists.txt
├── deps.lock.json
├── src/core/                  # hardware-independent deterministic core
├── tests/native/              # ordinary C++ tests
├── tests/python/              # repository/provenance validator tests
├── tools/                     # repository validators
├── docs/                      # architecture, POC sequence, test strategy, sources
├── firmware/field_truth/      # P0 contract; firmware implementation follows R0
├── hardware/                  # fixture notes and future measured mappings
├── evidence/                  # raw P0/P1 evidence, never simulated as "measured"
└── third_party/manifest.json  # source/provenance ledger
```

## Build and test R0

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python tools/validate_repo.py
python tools/validate_provenance.py
python -m unittest discover -s tests/python -v
```

Sanitizers on GCC/Clang:

```bash
cmake -S . -B build-sanitize -DBUILD_TESTING=ON -DHPF_ENABLE_SANITIZERS=ON
cmake --build build-sanitize --parallel
ctest --test-dir build-sanitize --output-on-failure
```

## Milestones

- **R0 - Repository foundation:** deterministic host core and automation. Implemented in this bootstrap.
- **P0 - Field Truth Test:** actual stock Field controls, audio, CV/Gate, MIDI, display/LED behavior, and callback timing measured on hardware.
- **P1 - Beat Core:** Hammer / Crack / Steel / Arc with deterministic sonic scenarios.
- **P2 - Performance layer:** A/B/Fill and direct live gestures.
- **P3 - Sequencing depth:** microtiming, ratchets, probability, parameter locks only after the direct workflow remains coherent.
- **P4 - Hydrasynth interaction:** MIDI/CV/clock/audio-through only after standalone proof.

See [docs/POC_SEQUENCE.md](docs/POC_SEQUENCE.md) for completion gates.

## Upstream boundaries

R0 does not vendor libDaisy or DaisySP. Exact upstream commits inspected for this bootstrap are recorded in `deps.lock.json` and `third_party/manifest.json`.

The source review also records useful architecture/test references found through `Denys/embedded-audio-mine`. Those references are evidence and design inputs, not proof that their implementation belongs in HydraPulse.

## Root license

No HydraPulse root license is selected in this bootstrap. That decision remains explicit rather than being silently inferred from dependencies or references.
