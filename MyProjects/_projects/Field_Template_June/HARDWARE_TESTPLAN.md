# Field Template June Automated Hardware Test Plan

This plan verifies `Field_Template_June` with the maximum practical unattended
coverage available from the repo and the Daisy Field USB serial interface. It
does not claim full physical validation from serial alone: serial can record
boot, firmware state, software invariants, MIDI/control counters, and parameter
snapshots, but it cannot rotate knobs, press Field keys, inspect the OLED, or
measure audio without external fixtures.

## Automated Runner

From this project directory:

```powershell
.\run_hardware_testplan.ps1 -Flash -CaptureSeconds 30
```

The runner performs these actions without prompts:

- builds the firmware with `make`;
- runs the Daisy QAE validator with UTF-8 output;
- optionally flashes through ST-Link with `make program`;
- auto-detects a USB serial port that emits `[FTJUNE]` telemetry, or uses
  `-Port COMx` when supplied;
- sends serial commands: `HELP`, `SELFTEST`, `STATUS`, and `SNAP`;
- records serial telemetry and command logs;
- writes a timestamped `summary.json` with pass, fail, skipped, or blocked
  status for each automated section.

Evidence is written to:

```text
hardware_evidence/<yyyyMMdd-HHmmss>/
```

Useful variants:

```powershell
.\run_hardware_testplan.ps1 -SkipBuild -NoSerial
.\run_hardware_testplan.ps1 -SkipBuild -Port COM7 -CaptureSeconds 20
.\run_hardware_testplan.ps1 -Flash -Port COM7 -CaptureSeconds 45
```

`-Flash` remains opt-in. Without it, the script tests whichever firmware is
already running on the attached Field.

## Firmware Serial Contract

The firmware starts non-blocking USB CDC logging and emits tagged lines:

```text
[FTJUNE] BOOT stage=...
[FTJUNE] SELFTEST reason=... result=PASS|FAIL
[FTJUNE] STATUS ...
[FTJUNE] SNAP ...
```

Supported host commands:

```text
HELP
SELFTEST
STATUS
SNAP
```

The firmware logs only from the main loop and startup path, not from the audio
callback. Periodic `STATUS` lines are rate-limited to one per second.

## Fully Automated Checks

These checks require only the build tools and, for serial checks, an attached
Field running this firmware:

1. Build succeeds.
2. Daisy QAE validator succeeds.
3. Optional ST-Link flash succeeds when `-Flash` is supplied.
4. USB serial emits the `[FTJUNE]` marker.
5. Boot telemetry is observed.
6. Built-in software self-test reports `result=PASS`.
7. Status telemetry is observed.
8. Snapshot telemetry is observed.

The built-in self-test verifies software invariants that can be checked without
moving hardware:

- until-touched threshold rejects no movement and accepts deliberate movement;
- OLED update period remains 50 ms;
- LED update period remains 16 ms;
- MIDI event cap remains 16 events per main-loop tick;
- boot defaults for main bank, alt bank, and output level match the intended
  template values.

## Observable During Capture

If a rig or user actuates controls while the runner is recording serial, the log
captures counters and snapshots that help diagnose these behaviors:

- MIDI note-on, note-off, and CC activity;
- parameter capture count and parameter write count;
- output-level capture/write count;
- Field key events;
- reset and panic events;
- active bank, gate, sustain, note, captured knob masks, and output capture
  state.

This makes the run useful for semi-automated bench tests. For example, a MIDI
source can play notes during `-CaptureSeconds 45`, and the resulting serial log
will show whether MIDI activity reached the firmware.

## Tests That Need External Fixtures

These are not fully automated by serial alone:

| Area | Why serial alone is insufficient | Automation path |
| --- | --- | --- |
| Physical knobs | USB serial cannot rotate analog controls. | Motorized fixture or manual movement during capture. |
| SW1/SW2 and Field keys | USB serial cannot press switches. | Relay/actuator fixture or manual action during capture. |
| OLED visibility | Firmware can report display update count, not optical readability. | Camera/OCR fixture or manual inspection. |
| Audio output | Firmware can report gate/parameter state, not actual output waveform. | Audio interface capture plus RMS/frequency checks. |
| MIDI DIN input | Firmware can count received MIDI only if a MIDI source is connected. | Scripted MIDI generator through a DIN/TRS interface. |

## Hardware-Acceptance Criteria

For unattended evidence, accept the run as `automation-pass` when:

- build is `PASS`;
- QAE is `PASS`;
- flash is `PASS` when requested, or `SKIPPED` when not requested;
- serial is `PASS`;
- `boot_marker`, `selftest_pass`, `status_seen`, and `snapshot_seen` are all
  `PASS` in `summary.json`.

For full hardware acceptance, add fixture or manual evidence for:

- external MIDI notes produce audio on both outputs;
- `SW1` switches main/alt bank editing without stored-value jumps;
- `SW2 + K8` edits hidden output level without changing `Color` or `Sub`;
- `B6`, `B7`, and `B8` reset behavior remains stable until physical knob
  movement crosses the touch threshold;
- `B5` panic clears sustain/gate and stops audio;
- OLED and LED updates remain readable during continuous control motion.
