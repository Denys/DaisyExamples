# P0 Field Truth firmware contract

This directory intentionally contains the P0 implementation contract before a target firmware claim is made.

## Verified libDaisy API surface at pinned R0 reference

At `libDaisy` commit `cc146d5065dd8286078a662e2830bf820c37a612`, `DaisyField` exposes the primitives needed for P0:

- `Init()`;
- `StartAudio()` / `StopAudio()`;
- `StartAdc()`;
- `ProcessAnalogControls()` / `ProcessDigitalControls()`;
- `KeyboardState()` / rising/falling edges;
- `GetKnobValue()`;
- `GetCvValue()`;
- `GetSwitch()`;
- `SetCvOut1()` / `SetCvOut2()`;
- public `gate_in`, `gate_out`, `midi`, `display`, `led_driver`, `seed`.

`DaisySeed` exposes `SetLed()`, `SetTestPoint()`, `StartLog()`, `Print()` and `PrintLine()`.

## Required P0 firmware behavior

The first implementation should:

1. stereo-pass audio in the callback without serial/display work in that callback;
2. collect bounded callback timing instrumentation for the full callback interval;
3. log raw 16-key scan indices outside the callback;
4. report eight knob and four CV normalized readings at a rate-limited interval;
5. report Gate In state/edges;
6. report received MIDI events;
7. provide OLED/LED functional indication outside the callback;
8. keep CV/Gate outputs in a defined safe state except during an explicit momentary output test;
9. emit machine-parseable telemetry such as `[HPF] key index=...`;
10. never claim a physical mapping until the bench log ties raw indices to physical keys.

## Completion

A successful ARM build will be recorded as build evidence only. P0 remains incomplete until `evidence/p0/` contains real hardware data for controls, I/O, audio, MIDI, and timing.
