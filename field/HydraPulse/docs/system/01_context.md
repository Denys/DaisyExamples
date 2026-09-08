# Context

The instrument and diagnostic are different firmware images. Beat ignores audio inputs and keeps
both DAC codes and Gate Out at zero/false. Truth provides the measurement path.
No MIDI parser, display writer or logger has a reference to Engine in a concurrent context:
the foreground sends value-only commands, and receives value-only snapshots.
The single audio owner is the primary race-avoidance mechanism. The two SPSC queues have
one producer and one consumer each, reject overflow and never overwrite unread slots.
Source: firmware/HydraPulse.cpp, firmware/FieldTruth.cpp, src/app/Engine.*.

```mermaid
flowchart LR
    Keys["Field: 16 keys + SW1/SW2"] --> Scan["ReadControls / 1 kHz"]
    Knobs["Field: 8 knobs"] --> Scan
    Scan --> Controller["Controller::Tick"]
    Controller --> Engine["Engine / single audio owner"]
    Midi["Field MIDI UART receiver"] --> Foreground["Foreground MIDI policy"]
    Foreground --> Queue["SpscQueue: 31 commands"]
    Queue --> Engine
    Foreground --> Urgent["Atomic urgent stop flag"]
    Urgent --> Engine
    Engine --> Voices["Hammer / Crack / Steel / Arc"]
    Voices --> Out["DC block / saturation / master ramp"]
    Out --> Codec["Field stereo output"]
    Engine --> Snap["SpscQueue: 7 snapshots"]
    Snap --> Display["Foreground OLED + LEDs + serial"]
    CV["CV and Gate ports"] -. "diagnostic target only" .-> Truth["FieldTruth"]
    AudioIn["Stereo audio input"] --> Truth
    Truth --> Passthrough["Unprocessed stereo passthrough"]
    Passthrough --> Codec
    Hydra["Hydrasynth-specific integration"] -. "deferred; no path in Beat" .-> Later["Later milestone"]
```
