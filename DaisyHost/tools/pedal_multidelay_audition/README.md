# Pedal Multi-Delay Audition Runner

This bounded host tool renders and profiles the current `PedalDelayEngine`
without involving JUCE, the DaisyHost editor, the live plugin processor, or a
Daisy target adapter.

Its purpose is to create a deterministic **host audition package** for the five
compact pedal modes:

- `DIGI`
- `TAPE`
- `MOD`
- `REV`
- `FREEZE`

Each mode has three cells in
`DaisyHost/training/examples/pedal_multidelay_audition_matrix.csv`:

- `reference`
- `character`
- `stress`

The generated package contains 15 stereo float WAV files, one JSON evidence
record per cell, `audition_manifest.csv`, `host_profile.csv`, and
`host_profile_environment.json`.

## Build and run

From the repository root:

```sh
cmake -S DaisyHost/tools/pedal_multidelay_audition \
      -B DaisyHost/build-audition \
      -DCMAKE_BUILD_TYPE=Release
cmake --build DaisyHost/build-audition --config Release
ctest --test-dir DaisyHost/build-audition \
      -C Release \
      --output-on-failure
```

Or invoke the executable directly:

```sh
DaisyHost/build-audition/pedal_multidelay_audition \
  --matrix DaisyHost/training/examples/pedal_multidelay_audition_matrix.csv \
  --output-dir DaisyHost/build-audition/audition_artifacts
```

The exact executable path is generator-dependent on multi-config platforms.

## Render evidence

The sources are deterministic synthetic engineering signals, not licensed
instrument performances and not disguised guitar listening tests:

- `pluck_train`: repeated decaying noise/harmonic excitation;
- `single_pluck`: one decaying excitation followed by silence;
- `transient_train`: alternating narrow transients;
- `sine`: steady 220 Hz diagnostic tone;
- `dual_tone`: deterministic 165/247.5 Hz diagnostic pair.

A render cell passes only when:

- every output sample is finite;
- output is non-silent when non-silence is expected;
- peak magnitude remains below the bounded host sanity limit;
- freeze reference/character cells retain measurable tail energy;
- WAV and machine-readable evidence are emitted.

These checks can reject broken output. They do not certify musical quality.

## Host profiling contract

The profiler measures the engine-only block call using
`std::chrono::steady_clock` in Release configuration.

```text
block_budget_us =
    1'000'000 * block_frames_per_channel / sample_rate_hz

margin_p99_9_us =
    block_budget_us - p99_9_us

tail_utilization_ratio =
    p99_9_us / block_budget_us

deadline_miss_count =
    count(observed_block_us > block_budget_us)
```

Per mode it records:

- arithmetic mean;
- p99;
- p99.9;
- observed maximum, explicitly not WCET;
- p99.9 margin and utilization;
- observed deadline misses and maximum lateness;
- warm-up count, measured count, measured audio duration;
- timer-bracket overhead and clock identity;
- non-finite processing failures.

The bounded capture uses 1,000 warm-up blocks and 50,000 measured blocks.
Percentiles are computed after collection with `std::nth_element`; no sorting,
formatting, file I/O, logging, or container growth occurs inside the measured
engine call.

Allowed labels are:

- `HOST_OBSERVED_WITHIN_BUDGET`
- `HOST_OBSERVED_TAIL_VIOLATION`
- `INSUFFICIENT_SAMPLE_COUNT`
- `HOST_MEASUREMENT_INVALID`

None means `SAFE_REAL_TIME`.

## Evidence boundary

A successful run may report:

```text
HOST_AUDITION_PACKAGE_READY
HOST_LISTENING_PENDING
TARGET_TIMING_NOT_RUN
```

It may not report:

- `HOST_LISTENED_ACCEPTED` without a recorded human listening session;
- Cortex-M7 timing or callback margin;
- target SDRAM/cache behavior;
- target-build or firmware-toolchain compatibility;
- physical audio, controls, electrical, mechanical, or production readiness.

`PedalDelayEngine` remains a DaisyHost audition/reference implementation.
Current portable product and target authority remains in the originating
portable-DSP/firmware repositories.

## Current real-time safety hold

This runner calls `PedalDelayEngine` directly and therefore does not exercise
the `PedalDelayCore` host wrapper. At the inspected baseline, the wrapper has a
scratch-buffer grow fallback in `Process()` when `frameCount` exceeds the
prepared capacity. Until that shared wrapper path is coordinated, repaired,
and tested, use:

```text
ENGINE_PROCESS_ALLOCATION_FREE: VERIFIED_SOURCE
CORE_WRAPPER_ALLOCATION_GUARANTEE: HOLD
```

The runner does not launder that known wrapper gap into an allocation-free
claim for the complete live DaisyHost path. A spreadsheet could probably be
persuaded to do so, but this tool has standards.
