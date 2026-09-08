# Preset Transaction

Presets are generated from presets/tutorials.json by tools/generate_presets.py.
The JSON is the editable source; the checked-in C++ table is a build asset whose exact
content is checked. There are eight presets with three banks and four voices each.
The preset contains rhythm masks, accents, sound parameters, BPM, swing and drive.
It does not contain master volume, which remains under the user's control.

There is no persistent save in this version. Edits live in RAM and are lost at power-down,
reset or confirmed preset load. No blank "save" menu implies otherwise.
The renderer exercises A/B, Fill and panic within each tutorial's ten-second host scenario.
Later storage requires a versioned schema, CRC, bounds checks, two-slot atomic commit,
power-interruption tests and a non-audio owner for flash operations.

```mermaid
sequenceDiagram
    participant User
    participant UI as Controller
    participant E as Engine
    participant DSP as Voice and mix state
    User->>UI: Shift + previous/next while stopped
    UI->>UI: Preview only / five-second timeout
    User->>UI: Shift + load
    UI->>E: RequestPreset(id)
    E->>E: Reject when running, busy or id invalid
    E->>DSP: Ramp load gain to zero over 5 ms
    E->>DSP: At zero: reset voices and DC state
    E->>E: Copy A/B/Fill and parameter table
    E->>DSP: Ramp load gain back to one
    E-->>UI: PresetRevision increments
    UI->>UI: Re-arm pickups against current physical knobs
    Note over E,DSP: Master is retained, transport remains stopped
```
