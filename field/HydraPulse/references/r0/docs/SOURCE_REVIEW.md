# Source review used for R0 bootstrap

Date: 2026-08-16

This review records sources actually inspected before creating the bootstrap. `Denys/embedded-audio-mine` was used as a discovery index; upstream repositories were then inspected directly for concrete files/APIs.

## Repository state

### `Denys/HydraPulse-Field`

- configured default branch: `main`;
- repository visibility observed through the connected GitHub app: private;
- GitHub contents API returned: **repository is empty**;
- no branch ref / HEAD commit exists yet, so there is no main SHA to preserve.

### `Denys/embedded-audio-mine`

Inspected commit:

`1113f0c6255c24a6655be968fb5aa115b8d5f7e9`

Its README explicitly separates published digests, selected projects, and anti-repeat evidence. A selected project is not automatically a reusable dependency. That distinction is retained here.

## Official Daisy sources

### `electro-smith/libDaisy`

Inspected commit:

`cc146d5065dd8286078a662e2830bf820c37a612`

Verified public Field API in `src/daisy_field.h` includes:

- eight `KNOB_*` entries;
- four `CV_*` entries;
- 16-key keyboard state/rising/falling edge functions;
- `SetCvOut1()` and `SetCvOut2()`;
- `gate_in` and `gate_out`;
- `midi`;
- `display`;
- Field LED driver;
- audio/ADC lifecycle methods.

`src/daisy_seed.h` also exposes `SetLed()`, `SetTestPoint()`, `StartLog()`, `Print()` and `PrintLine()`.

`src/hid/gatein.h` exposes both edge-style `Trig()` and current `State()`.

### `electro-smith/DaisyExamples`

Inspected commit:

`259ed82c7d4c0d7699f695945e6e111b7dc7cb27`

Verified examples:

- `field/KeyboardTest/KeyboardTest.cpp` uses `DaisyField`, 16 keyboard states, eight knobs, four CV inputs, LED driver and DAC output;
- `field/Midi/Midi.cpp` uses `hw.midi.StartReceive()`, `Listen()`, `HasEvents()` and `PopEvent()`;
- `field/KeyboardTest/Makefile` shows the conventional libDaisy/DaisySP relative layout and core Makefile include.

These are the preferred starting references for P0 because they exercise the actual target BSP.

### `electro-smith/DaisySP`

Inspected commit:

`599511b740f8f3a9b8db72a0642aa45b8a23c3a3`

Reserved for P1. R0 does not depend on DaisySP.

## Embedded Audio Mine leads reverified upstream

### `bkshepherd/DaisySeedProjects`

Inspected commit:

`80feee11f26a401ae324de75d9266e63ed82deb2`

Verified `BaseHardwareModule` API includes audio lifecycle, control processing, hardware capability queries, MIDI/display members, and protected initialization helpers. `GuitarPedal125B` derives from that base and overrides `Init()`.

**Borrowed idea:** keep hardware-facing details behind a stable adapter rather than allowing application semantics to spread through BSP calls.

### `hasanalpdoyduk/Teensy_Drum_Machine`

Inspected commit:

`187393d087a406134deee944edcd400aad7c7367`

Verified repository tree contains separate `ClockEngine`, `InstrumentManager`, `StorageManager`, `MidiEngine`, input managers and `Sequencer`. `Firmware/Sequencer.h` exposes direct `toggleStep`, `setStep`, `advanceStep`, `resetStep`, and selected-instrument state around a fixed pattern array.

**Borrowed idea:** direct 16-step semantics plus clear subsystem boundaries. HydraPulse core code was written independently for a fixed-size, libDaisy-free host-testable implementation.

### `kencul/STM32F4-AudioSynthesis`

Inspected commit:

`b1de88d6a3967327cc8a0eeccba1ebad524ccbd2`

Verified `.github/workflows/tests.yml` configures/builds/runs `tests` with CMake/CTest. `tests/CMakeLists.txt` explicitly compiles hardware-independent DSP sources on host while excluding HAL/hardware dependencies. The target application also uses Cortex DWT cycle-counter instrumentation.

**Borrowed idea:** host CMake/CTest for pure DSP/core plus separate target timing instrumentation.

## Not imported into R0

No community project source file is copied wholesale into the R0 core. The source ledger preserves exact upstream commits and the architectural concepts used.

License constraints were explicitly disabled by the user for this bootstrap turn; provenance was still retained so later release policy can be re-enabled without reconstructing history.
