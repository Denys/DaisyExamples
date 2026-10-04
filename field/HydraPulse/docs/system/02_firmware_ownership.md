# Firmware Ownership

Expected firmware sample rate is 48,000 Hz, block length is 48 frames and scan rate is 1,000 Hz.
These are configuration values, not measured physical clock accuracy. A snapshot is offered
every 40 callbacks (25 Hz). The foreground consumes at most eight per iteration and displays
only the last. Normal MIDI command consumption is at most eight per callback; urgent stop
does not wait behind note traffic.
CpuLoadMeter brackets callback work and uses System ticks, not Cortex cycle counts.
The testpoint pulse includes the user callback body but excludes library interrupt entry,
sample conversion outside the callback and exit overhead. Late-start and overrun counters
are indicators, not a complete DMA underrun detector. Measure the instrumented interval
and interrupt/codec behavior on the actual hardware before interpreting headroom.

```mermaid
sequenceDiagram
    participant FG as Foreground
    participant MIDI as MIDI driver
    participant Q as Command queue
    participant IRQ as Audio callback
    participant UI as Controller
    participant E as Engine
    participant S as Snapshot queue
    participant OLED as OLED and LED drivers
    FG->>MIDI: Listen / pop at most 32 events per iteration
    FG->>Q: Push supported command or count drop
    IRQ->>IRQ: Testpoint high / meter begin
    IRQ->>UI: Scan controls and Tick once
    IRQ->>Q: Drain at most 8 normal commands
    Note over IRQ,E: Urgent stop instead flushes at most 32 stale commands
    IRQ->>E: Process each of 48 samples
    E-->>IRQ: StereoFrame
    IRQ->>S: Every 40 blocks: copy UiSnapshot
    IRQ->>IRQ: Meter end / testpoint low
    FG->>S: Drain at most 8 snapshots
    FG->>OLED: Draw most recent snapshot
    FG->>FG: Rate-limited serial telemetry
```
