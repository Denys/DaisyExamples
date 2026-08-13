# Real-Time DSP Profiling System — Technical Specification

Version: `1.1-candidate`  
Status: `IMPLEMENTED_HOST_SUBSET / TARGET_EVIDENCE_NOT_RUN`  
Primary use: embedded-audio callback envelope characterization  
Current host implementation: `DaisyHost/tools/pedal_multidelay_audition/`

## 0. Executive contract

This specification defines a bounded execution-time measurement and
interpretation system for real-time audio DSP.

It separates four things that must not be collapsed into one reassuringly
vague percentage:

1. measured host execution time;
2. measured target execution time;
3. observed deadline violations;
4. explicitly modeled sensitivity scenarios.

The system estimates an **observed execution envelope** for a declared build,
graph, mode, input and stress profile. It does not prove WCET, cycle-accurate
MCU equivalence, audio quality, or production readiness.

## 0.1 Canonical units and names

```text
sample_rate_hz
block_frames_per_channel
block_budget_us
mean_exec_us
p99_exec_us
p99_9_exec_us
observed_max_exec_us
p99_9_margin_us
observed_max_margin_us
p99_9_load
observed_max_load
exec_deadline_miss_count
exec_deadline_miss_rate
max_lateness_us
```

`buffer_size` is prohibited in timing formulas because it is ambiguous between
bytes, aggregate samples and frames per channel.

## 0.2 Callback budget

```text
block_period_s =
    block_frames_per_channel / sample_rate_hz

block_budget_us =
    1'000'000.0 * block_frames_per_channel / sample_rate_hz
```

The channel count does not multiply the available time. Stereo increases the
work, not the callback period.

Example:

```text
sample_rate_hz          = 48'000
block_frames_per_channel = 48
block_budget_us          = 1'000
```

## 0.3 Primary statistics

```text
mean_exec_us =
    arithmetic mean of all valid post-warm-up observations

p99_exec_us =
    observed 99.0th percentile

p99_9_exec_us =
    observed 99.9th percentile

observed_max_exec_us =
    largest valid observation in the bounded campaign
```

`observed_max_exec_us` is explicitly **not WCET**. It remains important:
when it exceeds the budget, an actual deadline violation was observed.

## 0.4 Margins and loads

```text
p99_9_margin_us =
    block_budget_us - p99_9_exec_us

observed_max_margin_us =
    block_budget_us - observed_max_exec_us

p99_9_load =
    p99_9_exec_us / block_budget_us

observed_max_load =
    observed_max_exec_us / block_budget_us
```

`p99_9_load` is a utilization ratio. It is not a probability and must not be
named `deadline_risk`.

## 0.5 Deadline observations

```text
exec_deadline_miss_count =
    count(exec_us > block_budget_us)

exec_deadline_miss_rate =
    exec_deadline_miss_count / valid_observation_count

max_lateness_us =
    max(0.0, observed_max_exec_us - block_budget_us)
```

Where a target/driver surface exposes them, record separately:

```text
callback_interval_overrun_count
reported_driver_xrun_count
processing_failure_count
```

These counters describe different failure mechanisms and may not be merged.

## 0.6 Allowed interpretation labels

Measured host results may use:

- `HOST_OBSERVED_WITHIN_BUDGET`
- `HOST_OBSERVED_TAIL_VIOLATION`
- `HOST_REGRESSION_PASS`
- `HOST_REGRESSION_FAIL`
- `INSUFFICIENT_SAMPLE_COUNT`
- `HOST_MEASUREMENT_INVALID`

Measured target results may use:

- `TARGET_OBSERVED_WITHIN_CAMPAIGN_GATE`
- `TARGET_OBSERVED_GATE_VIOLATION`
- `TARGET_MEASUREMENT_INVALID`
- `TARGET_NOT_RUN`

Prohibited conclusions from a finite campaign:

- `SAFE_REAL_TIME`
- `WCET_PROVEN`
- `DETERMINISTICALLY_SAFE`
- `TARGET_READY` from host evidence

## 1. Scope

The profiler supports:

- block-based real-time audio DSP;
- host regression and relative-cost comparison;
- physical-target callback measurement;
- mode/state/transition-aware campaigns;
- bounded report generation;
- optional, separately labeled sensitivity projections.

It evaluates:

- execution-time distribution for the declared campaign;
- tail proximity to the callback budget;
- observed deadline misses;
- sensitivity to declared stress conditions;
- regressions between comparable revisions.

## 2. Timing sources

### 2.1 Host

Host measurement shall use:

```text
std::chrono::steady_clock
```

Record:

- `steady_clock::is_steady`;
- nominal clock period;
- measured empty-bracket overhead;
- operating system;
- CPU identity where available;
- compiler and version;
- build configuration and optimization flags;
- process/thread-priority policy or default status.

`std::chrono::high_resolution_clock` is prohibited unless the implementation
first proves that it aliases a monotonic clock on the measured platform. The
portable default remains `steady_clock`.

### 2.2 Cortex-M and other embedded targets

Target measurement shall use a qualified monotonic hardware source, such as:

- ARM DWT `CYCCNT`;
- a verified general-purpose hardware timer;
- a target RTOS trace/timestamp source with documented resolution.

Record:

- counter source and width;
- core/timer frequency;
- wrap handling;
- read overhead;
- cache state;
- memory placement;
- interrupt and measurement scope.

The target shall not be forced through `std::chrono` merely to make host and
firmware code look aesthetically symmetrical.

## 3. Measurement scope

Every campaign shall state exactly what the timing bracket includes.

Possible scopes include:

- DSP engine call only;
- complete product graph;
- complete audio callback body;
- callback response time including observed scheduling delay;
- driver-reported period/xrun behavior.

A DSP-engine bracket does not include ISR entry/exit, DMA/driver work,
concurrent interrupts or board services unless explicitly placed inside the
bracket.

## 4. Warm-up and capture

Use callback blocks, not audio frames, as the warm-up unit.

Default host campaign:

```text
warmup_blocks   = 1'000
measured_blocks = 50'000
```

Minimum accepted warm-up:

```text
warmup_blocks >= 200
```

The campaign may use a longer warm-up or a declared stabilization criterion.
The report shall include:

- warm-up blocks;
- measured blocks;
- valid/invalid observations;
- measured audio duration;
- elapsed wall duration where useful;
- dropped/overwritten sample count.

A reported p99.9 requires at least 50,000 valid observations unless the report
explicitly returns `INSUFFICIENT_SAMPLE_COUNT`. At 50,000 observations, the
upper 0.1% still contains only approximately 50 samples; p99.9 remains an
observed statistic, not a worst-case theorem.

## 5. Bounded acquisition

The measured real-time path may perform only bounded work required to capture
timestamps and counters.

Allowed in the measured path:

- read monotonic timer/counter;
- process audio;
- read timer/counter;
- store one value into preallocated storage;
- update bounded counters/sum/maximum.

Prohibited in the measured path:

- allocation or deallocation;
- container growth;
- sorting or percentile computation;
- string construction or formatting;
- file/network/device I/O;
- logging;
- locks or blocking calls;
- unbounded loops.

Capture storage may be:

- one fixed-capacity finite campaign buffer; or
- a fixed-capacity ring buffer with explicit overwrite accounting.

Unbounded accumulation is prohibited.

## 6. Statistical computation

### 6.1 Mean

The report mean shall be an arithmetic mean derived from exact sum/count or a
numerically stable online equivalent.

An EMA may be exposed separately for live telemetry only when its alpha and
initialization are recorded. An EMA may not silently replace the campaign
mean.

### 6.2 Percentiles

The canonical percentile convention is nearest rank:

```text
index(q, N) = ceil(q * N) - 1
```

with zero-based clamping to `[0, N - 1]`.

Compute p99 and p99.9 after capture using `std::nth_element` or an equivalent
bounded partial-selection method. Full sorting is permitted outside the audio
path but is not required.

The report shall state:

- quantile;
- rank convention;
- valid sample count;
- samples above the selected quantile.

## 7. Decision model

### 7.1 Observed failure

Return an observed failure when any applicable condition is true:

```text
processing_failure_count > 0
OR exec_deadline_miss_count > 0
OR callback_interval_overrun_count > 0
OR reported_driver_xrun_count > 0
OR invalid/non-finite timing observations > 0
```

A system may also fail a stricter, predeclared margin gate before any deadline
miss occurs.

### 7.2 Campaign pass

A campaign may return an observed pass only when:

- exact build/graph/input/stress identity is recorded;
- observation count and duration satisfy the declared campaign;
- timing source and overhead are valid;
- zero applicable failure counters are observed;
- p99.9 and maximum satisfy the predeclared campaign limits;
- every required mode/state/transition cell is executed.

The result is scoped to that campaign.

### 7.3 Project provisional target gate

Where the active product campaign has not superseded it, the provisional
Daisy gate is:

```text
p99_9_exec_us <= 0.70 * block_budget_us
observed_max_exec_us <= 0.85 * block_budget_us
all observed overrun/xrun/failure counts == 0
```

This gate is a product-program policy, not a universal real-time theorem.

## 8. WCET boundary

Do not infer WCET from:

- one maximum observation;
- p99 or p99.9;
- p99.9 plus an arbitrary constant;
- host-to-target multipliers.

An optional policy estimate may be reported as:

```text
tail_bound_proxy_us =
    p99_9_exec_us + configured_tail_margin_us
```

or:

```text
tail_bound_proxy_us =
    p99_9_exec_us * configured_tail_factor
```

Required metadata:

- policy id;
- absolute margin/factor;
- calibration source;
- applicable target and stress profile.

Required label:

```text
TAIL_BOUND_PROXY_NOT_WCET
```

## 9. Host sensitivity model

A modeled layer is optional and disabled by default.

Its name is:

```text
HOST_SENSITIVITY_MODEL
```

It must not be called embedded simulation or target emulation.

Measured and modeled fields shall remain separate:

```text
measured_exec_us
modeled_exec_us
modeled_scheduling_delay_us
modeled_available_budget_us
model_configuration
model_provenance
```

Example structure:

```text
modeled_exec_us =
    measured_exec_us * calibrated_execution_factor

modeled_available_budget_us =
    block_budget_us
    - max(0, modeled_scheduling_delay_us)
    - reserved_system_budget_us
```

Every non-default model requires:

- explicit factor/range;
- event probability and distribution;
- burst/correlation model;
- deterministic seed;
- calibration source;
- applicable memory/cache/ISR condition.

All uncalibrated output shall be labeled:

```text
UNCALIBRATED_SCENARIO_PROJECTION
```

Cache and memory-contention penalties shall not be multiplied as independent
random variables when evidence indicates they are correlated.

## 10. Mode- and state-aware campaigns

Profile product behavior by mode and by transient state. For the compact
multi-delay product, minimum target-facing coverage eventually includes:

```text
DIGI steady state
DIGI active TIME transition
TAPE steady state and maximum transport modulation
MOD maximum rate/depth/resonance
REV overlapping grain/window stress
FREEZE capture
FREEZE hold/evolve
FREEZE accumulate/replace/clear transitions
mode transition
bypass/trails transition
representative background control/MIDI/UI load
```

The highest cost may occur during a bounded transition where two policies or
read heads execute together. A per-mode steady-state average is therefore
insufficient.

## 11. Required report metadata

Every report shall include:

### Identity

- repository and commit;
- dirty/clean state where observable;
- build id;
- compiler/version/flags;
- board/host identity;
- graph and mode/state;
- source/input vector and seed;
- scenario id.

### Runtime

- sample rate;
- block frames per channel;
- channel count;
- callback budget;
- clock/counter source and overhead;
- memory placement and cache state where applicable;
- warm-up/measured blocks;
- measured duration.

### Metrics

- mean;
- p99;
- p99.9;
- observed maximum;
- p99.9 and maximum margins/loads;
- execution deadline misses/rate/lateness;
- callback interval overruns;
- driver xruns;
- processing failures;
- invalid/non-finite observations.

### Model boundary

- sensitivity model enabled/disabled;
- complete model configuration if enabled;
- target profiling performed/not performed;
- tests and measurements not run.

## 12. Current DaisyHost implementation mapping

`DaisyHost/tools/pedal_multidelay_audition/` implements the host subset:

- `std::chrono::steady_clock`;
- timer-overhead record;
- 1,000 warm-up blocks;
- 50,000 measured blocks per mode;
- preallocated bounded duration buffer;
- arithmetic mean;
- nearest-rank p99 and p99.9 via `std::nth_element`;
- observed maximum;
- execution deadline-miss count/rate/lateness;
- non-finite processing-failure count;
- per-mode CSV plus environment JSON;
- sensitivity model disabled;
- explicit `TARGET_TIMING_NOT_RUN`.

Not implemented in that host runner:

- callback-start interval timing;
- driver xrun reporting;
- CPU affinity or priority control;
- physical target counters;
- SDRAM/cache measurement;
- calibrated host sensitivity projection;
- WCET analysis.

## 13. Non-goals

The profiling system shall not:

- estimate audio quality;
- replace listening or calibrated audio measurement;
- replace target profiling tools;
- assume cycle-accurate MCU simulation;
- use CPU percentage as the primary metric;
- treat p99.9 or observed maximum as WCET;
- hide observed misses behind a percentile;
- perform unbounded statistical accumulation;
- convert host nanoseconds into Daisy callback margin;
- authorize product/production readiness.

## 14. Completion gate

A profiler implementation is acceptable for its declared surface only when:

- formulas and units match this specification;
- timing source is monotonic and recorded;
- acquisition is bounded and real-time safe for the measured scope;
- percentile convention and sample sufficiency are explicit;
- deadline misses and invalid observations are counted;
- measured and modeled results remain separate;
- host and target evidence are not conflated;
- deterministic tests validate metric calculation and invalid-input behavior;
- performed and unperformed checks are visible.

Current disposition:

```text
HOST_SUBSET: IMPLEMENTED_AND_CI_EXECUTED
TARGET_PROFILER: SEPARATE_EXISTING_CANDIDATE
PHYSICAL_TARGET_RUN: NOT_RUN_IN_THIS_WORK_PACKAGE
WCET: UNKNOWN
```
