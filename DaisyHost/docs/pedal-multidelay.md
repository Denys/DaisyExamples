# Pedal Multi-Delay (`pedal_multidelay`)

The compact multi-delay guitar pedal running inside DaisyHost, so the algorithms
and the control surface can be auditioned and tuned on the desktop without
flashing Daisy hardware for every iteration.

**Host status only.** Nothing here is a Daisy/ARM claim. No target build, no
Cortex-M7 timing, no SDRAM/cache measurement, and no hardware audio was run.

## Where it lives

| Concern | File |
|---|---|
| Portable DSP engine | [`include/daisyhost/PedalDelayEngine.h`](../include/daisyhost/PedalDelayEngine.h), [`src/PedalDelayEngine.cpp`](../src/PedalDelayEngine.cpp) |
| Host app wrapper | [`include/daisyhost/apps/PedalDelayCore.h`](../include/daisyhost/apps/PedalDelayCore.h), [`src/apps/PedalDelayCore.cpp`](../src/apps/PedalDelayCore.cpp) |
| Registration | [`src/AppRegistry.cpp`](../src/AppRegistry.cpp) |
| Tests | [`tests/test_pedal_delay_core.cpp`](../tests/test_pedal_delay_core.cpp) |
| Render scenario | [`training/examples/pedal_multidelay_smoke.json`](../training/examples/pedal_multidelay_smoke.json) |

The engine is deliberately host-agnostic: it takes external sample storage,
allocates nothing in `Process()`, takes no locks, does no I/O, and derives every
constant from the runtime sample rate. `PedalDelayCore` owns the storage and the
host-facing descriptors.

## Build and run

```bash
cmake -S . -B build
```

```bash
cmake --build build --config Release --target unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
```

```bash
ctest --test-dir build -C Release --output-on-failure
```

Inspect the live control map, including engineering labels and DSP targets:

```bash
./build/Release/DaisyHostCLI describe-app pedal_multidelay --json
```

Render five seconds of audio that walks through all five modes:

```bash
./build/Release/DaisyHostCLI render training/examples/pedal_multidelay_smoke.json --output-dir build/cli_smoke/pedal_multidelay --expect-non-silent --json
```

## Control surface

Five performance slots plus one navigation control, one active mode at a time.
Each slot carries a musician label and an engineering label; macros enumerate
every DSP target they touch. All ranges are **provisional** host tuning values:
current repository sources do not freeze pedal-facing ranges.

| Mode | TIME | FEEDBACK | MIX | COLOR | MOTION |
|---|---|---|---|---|---|
| DIGI | TIME - fractional read delay, ms | REPEATS - loop gain, ratio | MIX - wet/dry, ratio | BRIGHT - feedback LPF cutoff, Hz | DRIFT - read modulation depth, ms |
| TAPE | TIME - nominal transport delay, ms | REPEATS - conditioned loop gain, ratio | MIX - wet/dry, ratio | AGE - bandwidth loss + saturation macro | WARBLE - drift/wow/flutter macro |
| MOD | BASE TIME - base comb delay, ms | RESONANCE - signed comb coefficient | MIX - blend/feedforward pair | DEPTH - modulation depth, ms | RATE - LFO rate, Hz |
| REV | SLICE - reverse grain duration, ms | REPEATS - post-OLA re-injection gain | MIX - wet/dry, ratio | REVERSE - forward/reverse crossfade | GRAIN - window taper fraction |
| FREEZE | LOOP - capture loop length, ms | DECAY - recirculation gain | LEVEL - freeze output gain | DAMPING - loop LPF/HPF macro | EVOLVE - slow drift macro |

Navigation and footswitches:

- `node0/param/mode` selects the algorithm, no restart and no recompile;
- `node0/param/bypass` and `node0/param/trails` implement the bypass policy;
- `node0/param/tap` is tap tempo for the delay modes and the freeze hold toggle
  in FREEZE. The gate input drives the same event;
- `node0/param/freeze_state` and the `switches` menu section drive
  `capture / hold / accumulate / replace / clear` explicitly, for verification.

On Daisy Field all five slots sit on knobs 1..5, key row A is bypass / tap /
trails / the five modes, and key row B is the five freeze state operations. On
Daisy Patch the first four slots sit on the knobs and MOTION is reached through
the encoder menu.

## Documented DSP policies

**Fractional read.** Linear interpolation between the two neighbouring history
samples, the same policy as `DaisyDelayFxCore::DelayLine::Read`. Measured on
host: an impulse lands on exactly two taps, total gain 1.00000, delivered delay
matches the requested delay to better than 0.05 samples at 960 / 4800 / 9600
samples.

Two systematic errors were found and closed while measuring this:

- the normalized-to-engineering log mapping ran in `float`; `powf` alone costs
  about 1e-4 relative, so it now runs in double at control rate (the audio-rate
  variant stays single precision);
- the parameter smoother used the textbook `x += (t - x) * c`, which stalls in
  float32 once `(t - x) * c` falls below the ULP of `x`. That left a systematic
  0.011% delay-time offset, about one sample at 200 ms. The smoother now keeps
  the remaining error as its state and decays that instead, so it retains full
  relative precision and lands exactly on the target. Same cost per sample.

**Delay-time transition.** A one-pole slew of the read position, i.e. a
pitch-warp transition, never an instantaneous pointer jump. The TIME slot's
`smoothing_ms` *is* the transition constant: 60 ms in DIGI, 220 ms in TAPE where
the transport inertia is the point, 120 ms for the REV grain length. A host test
asserts the output envelope stays continuous through a 120 ms -> 900 ms jump.

**Mix law.** Linear crossfade, matching `DaisyDelayFxCore::Mix`. MOD is the
exception: its MIX slot is the comb blend/feedforward pair, so the output
crossfade is bypassed for that mode.

**Feedback conditioning.** One-pole high-cut in the loop for every mode; TAPE
adds a high-pass and a normalized `tanh` soft saturation after it. Loop gain is
clamped below unity (0.95 max, 0.85 for REV re-injection).

**Reverse readout.** Continuous writing plus two backward-reading grains half a
period apart. The reverse head walks away from the write head at twice the write
rate, so the absolute read position moves backwards at 1x speed. Grains are
summed and divided by the summed window, so the overlap-add gain is bounded for
any taper; a host test feeds DC and asserts the output stays inside
[0.9, 1.05] for every GRAIN setting. Algorithmic latency is one grain and is
reported by `GetAlgorithmicLatencySamples()`.

**Freeze.** One recirculating loop with an explicit state machine. `capture` and
`replace` fill the loop and auto-advance to `hold`; `replace` first discards the
previous capture and its filter state. `hold` mutes fresh input entirely,
`accumulate` injects it at 0.5 gain, `clear` returns to a known empty state. The
loop write-back goes through a `tanh` limiter, so DECAY at 0.999 combined with
ACCUMULATE stays bounded.

**Mode change.** The shared history and the freeze loop survive a mode change,
so the existing tail keeps ringing. Conditioning filters and modulation phases
are cleared and the parameter smoothers snap, because the engineering units
differ between modes. No buffer is allocated or freed on a mode change.

**Bypass.** With trails on, bypass mutes the new input send and keeps the wet
path running, so the tail survives. With trails off the output is the dry signal
bit-for-bit while the engine state is preserved.

**Sanitation.** Non-finite slot values are rejected and the previous value is
kept; finite out-of-range values are clamped. Non-finite audio input and output
are zeroed. Filter states flush denormals.

## Host performance evidence

Measured by `PedalDelayHostPerformanceTest.RecordsBlockProcessTiming`, 48 kHz,
64-frame blocks, 2 channels, 10 s per mode, Release build:

| Mode | avg per block | worst per block | block budget |
|---|---|---|---|
| DIGI | 3.66 us | 16.40 us | 1333.33 us |
| TAPE | 9.21 us | 126.20 us | 1333.33 us |
| MOD | 6.85 us | 1039.50 us | 1333.33 us |
| REV | 7.38 us | 47.60 us | 1333.33 us |
| FREEZE | 6.98 us | 62.00 us | 1333.33 us |

Averages are stable across runs. The worst-case column is not: it tracks OS
scheduling on a shared desktop, not DSP cost, and moves by an order of
magnitude between runs of identical code. Only the averages are usable
evidence.

Host numbers only. They say nothing about Cortex-M7 cycles, SDRAM behaviour, or
cache behaviour.

## Known gaps

- Ranges, defaults and curves are `provisional=true`; none is product-frozen.
- Mono in, dual-mono out. True stereo, ping-pong and dual-mono orchestration
  are still open product decisions.
- FREEZE is a single recirculating loop, not the three-loop incommensurate bank.
- TAPE has no noise or dropout model; nothing is hidden inside AGE.
- `powf` runs per sample in the TAPE and FREEZE filter mapping. Fine on the
  host, a known target cost; see the `ponytail:` note in `PedalDelayEngine.cpp`.
- A crossfade-based TIME transition was not implemented as an alternative to the
  slew policy.
- No preset/context behaviour behind the `MODE/PRESET/CONTEXT` encoder.
