# Multi-Delay DSP / Algorithm Assembly Work Package

Version: `0.2-candidate`  
Primary lane: `Delay pedal / DaisyHost audition model / DSP behavior characterization`  
Secondary handoff: `portable-core and firmware-portability findings only`  
Host target: `DaisyHost/PedalDelayEngine`  
Target state: `HOST_AUDITION_PACKAGE_READY`

## Manager summary

This work package makes the existing five-mode pedal engine reproducibly
renderable and measurable on a desktop. It is intended to expose behavior for
engineering review and human audition without pretending that a Windows or
Linux timing result is a Cortex-M7 result.

The bounded implementation adds:

- 15 deterministic audition cells, three per mode;
- headless float-WAV and JSON evidence generation;
- finite/non-silent/tail sanity gates;
- bounded host timing capture with mean, p99, p99.9, maximum, margins and
  observed deadline misses;
- explicit host, listening, portability and target evidence boundaries.

It does not redesign the UI, migrate the product DSP, or claim a physical
pedal result.

## Source-authority boundary

`DaisyHost/PedalDelayEngine` is a host audition and behavioral-reference
implementation.

It is not the current portable product DSP authority and it is not the current
Daisy target runtime. Host behavior must not silently supersede, fork, or claim
parity with the portable implementation in the originating product DSP
repository.

Classify every useful host result as one of:

- `HOST_ONLY_REFERENCE`
- `BEHAVIOR_CONTRACT_CANDIDATE`
- `PORTABILITY_RISK`
- `PORTABLE_CORE_HANDOFF_REQUIRED`
- `REJECTED_FOR_TARGET_PATH`

Do not copy DaisyHost source into the portable repository in this work
package. Produce a bounded handoff instead.

## Public contract

Keep stable:

- app id `pedal_multidelay`;
- mode identities and order: `DIGI`, `TAPE`, `MOD`, `REV`, `FREEZE`;
- physical slot order: `TIME`, `FEEDBACK`, `MIX`, `COLOR`, `MOTION`;
- freeze operations: `capture`, `hold`, `accumulate`, `replace`, `clear`;
- bypass/trails and tap/hold semantics;
- canonical parameter ids;
- `PedalSlotDescriptor` schema and meaning.

No public interface change is required by this bounded runner.

## Audition-readiness meaning

`HOST_AUDITION_PACKAGE_READY` means:

- deterministic render definitions exist;
- all expected output is finite and non-silent;
- machine-readable metrics and checksums exist;
- output WAVs are ready for a human listening pass;
- listening instructions and evidence boundaries are explicit.

It does not mean that anyone listened.

`HOST_LISTENED_ACCEPTED` may be reported only after an actual listening
session records:

- listener;
- source material and exact rendered files;
- monitoring chain;
- level/configuration;
- observations and disposition.

Numeric analysis can reject a candidate. It cannot positively certify musical
quality.

## Audition matrix

The source-of-run matrix is:

```text
DaisyHost/training/examples/pedal_multidelay_audition_matrix.csv
```

Required cells:

```text
DIGI    reference / character / stress
TAPE    reference / character / stress
MOD     reference / character / stress
REV     reference / character / stress
FREEZE  reference / character / stress
```

Every row declares:

- unique `pedal_multidelay_audition_*` scenario id;
- mode and cell;
- deterministic synthetic source;
- duration;
- five normalized slot values;
- bounded parameter/freeze event sequence;
- whether retained freeze-tail energy is expected.

The runner records normalized and native values in the output manifest.

## Render acceptance

Per cell:

- sample rate: 48 kHz;
- block size: 48 frames per channel;
- output: stereo IEEE-float WAV;
- finite output required;
- non-silent output required where expected;
- bounded host peak sanity check;
- FNV-1a 64-bit deterministic checksum;
- peak, RMS, final-window RMS, maximum adjacent step and stereo-difference RMS;
- final freeze state when applicable.

The sources are engineering stimuli, not licensed recordings and not synthetic
signals mislabeled as guitar listening evidence.

## Correct host timing model

```text
block_budget_us =
    1'000'000.0 * block_frames_per_channel / sample_rate_hz

mean_us =
    arithmetic mean of post-warm-up observed blocks

p99_us =
    observed 99.0th percentile

p99_9_us =
    observed 99.9th percentile

max_us =
    observed maximum, NOT WCET

margin_p99_9_us =
    block_budget_us - p99_9_us

tail_utilization_ratio =
    p99_9_us / block_budget_us

deadline_miss_count =
    count(observed_block_us > block_budget_us)

deadline_miss_rate =
    deadline_miss_count / measured_block_count

max_lateness_us =
    max(0.0, max_us - block_budget_us)
```

`tail_utilization_ratio` is not a probability.

Do not label `p99_9_us < block_budget_us` as `SAFE_REAL_TIME`. Allowed host
labels are:

- `HOST_OBSERVED_WITHIN_BUDGET`
- `HOST_OBSERVED_TAIL_VIOLATION`
- `INSUFFICIENT_SAMPLE_COUNT`
- `HOST_MEASUREMENT_INVALID`

A host run is comparative/regression evidence only.

## Clock and bounded statistics

Host timing uses `std::chrono::steady_clock` and records:

- whether the clock reports monotonic behavior;
- nominal period;
- minimum empty timing-bracket overhead;
- compiler and host OS;
- sample rate, block size, warm-up count and measured count.

Target timing must use the target profiler selected by the authoritative
firmware implementation, such as DWT `CYCCNT`. Target profiling is `NOT_RUN`
in this work package.

The bounded host capture uses:

```text
warmup_blocks   = 1,000
measured_blocks = 50,000 per mode
```

Percentiles are computed after collection with `std::nth_element`. No
allocation, sorting, formatting, logging or file I/O occurs inside the measured
engine call.

## WCET and sensitivity language

No quantity in this host tool is WCET.

An optional future engineering estimate may be called:

```text
tail_bound_proxy_us = p99_9_us + configured_tail_margin_us
```

It must record the margin source and remain explicitly `NOT_WCET`.

Cache, contention or scheduling-delay injection, if later added, must be
reported separately as:

```text
HOST_SENSITIVITY_MODEL
UNCALIBRATED_SCENARIO_PROJECTION
```

It must never be merged into measured timing or called embedded simulation.
The current runner leaves this model disabled.

## Real-time safety exception

At the inspected baseline, `PedalDelayEngine::Process()` itself has external
storage and no heap-growth path. The live `PedalDelayCore::Process()` wrapper,
however, contains a scratch-vector grow fallback when a callback exceeds the
prepared block capacity.

This bounded work package does not edit the shared wrapper while other
DaisyHost surfaces may depend on it. Until a coordinated repair and negative
test are complete, report:

```text
ENGINE_PROCESS_ALLOCATION_FREE: VERIFIED_SOURCE
CORE_WRAPPER_ALLOCATION_GUARANTEE: HOLD
RT_SAFETY_EXCEPTION_REQUIRED: YES
```

Smallest safe follow-up:

1. preallocate wrapper scratch storage before audio starts;
2. store the prepared maximum block size;
3. fail closed for an oversized callback rather than grow a vector;
4. add a negative test proving no allocation/growth occurs in `Process()`;
5. rerun the focused and full DaisyHost host gates.

## Portability review

The current host engine uses an embedded-friendly data model in several areas:

- explicit external long-buffer ownership;
- fixed-size state arrays;
- float sample processing;
- bounded sample loops;
- deterministic reset;
- no JUCE dependency in the engine.

Record these portability risks separately:

- DaisyHost builds as C++17 while current Daisy firmware paths may use C++14;
- `std::clamp` and `std::string_view` require toolchain review;
- per-sample `sin`, `cos`, `pow`, `tanh` and `fmod` calls require target-cost
  profiling;
- host vectors and maps remain in wrappers outside the engine;
- host RAM behavior does not prove SDRAM/cache suitability.

Do not perform a speculative migration or sound-changing optimization before
actual target profiling.

## Generated evidence

A successful run emits:

```text
audition_artifacts/
  pedal_multidelay_audition_*.wav       # 15 files
  pedal_multidelay_audition_*.json      # 15 files
  audition_manifest.csv
  host_profile.csv
  host_profile_environment.json
```

The program prints:

```text
HOST_AUDITION_PACKAGE_READY
HOST_LISTENING_PENDING
TARGET_TIMING_NOT_RUN
```

## Verification commands

```sh
cmake -S DaisyHost/tools/pedal_multidelay_audition \
      -B DaisyHost/build-audition \
      -DCMAKE_BUILD_TYPE=Release
cmake --build DaisyHost/build-audition --config Release
ctest --test-dir DaisyHost/build-audition \
      -C Release \
      --output-on-failure
```

GitHub Actions runs the same bounded subproject and uploads the evidence
package.

## Not claimed

- human listening acceptance;
- complete live-wrapper allocation safety;
- full DaisyHost/JUCE/plugin gate from the bounded subproject alone;
- ARM build or target-toolchain compatibility;
- Cortex-M7 average, p99.9, maximum or overruns;
- SDRAM/cache timing;
- physical audio, controls, analog I/O, power, EMI/EMC or mechanics;
- product or production readiness.

## Completion disposition

This work package is `PASS` only for the bounded host audition/profiling tool
when all 15 cells and profiler-integrity checks pass.

The broader multi-delay workstream remains `PARTIAL` while:

- human listening is pending;
- the live wrapper allocation guarantee is `HOLD`;
- target profiling and physical audio remain `NOT_RUN`;
- portable-product parity remains unverified.
