# field_mixer

`field_mixer` is a Daisy Field performance mixer for two mono instruments:
Hydra on `IN L` and Edge on `IN R`. It keeps the sources separate at the
control level, then creates one common output mix on `OUT L` and `OUT R`.
The headphone output carries the same common mix because the Field exposes the
headphone jack through the same codec output pair used by the line outputs.

The project intentionally avoids side-image controls. Round 1 focuses on level,
input cleanup, no-input hum suppression, tone, delay-send performance, ducking,
saturation, and output safety.

## Concept

- Purpose: mix Hydra and Edge quickly in a small hardware rig without adding a
  deep effects workstation.
- Required DSP: DC blocking, high-pass filtering, smooth input gating for
  floating/no-input jacks, tone tilt, mono delay, envelope-based Edge ducking,
  optional soft saturation, and always-on limiting.
- Complexity rating: 5/10. The DSP is deliberately conservative; the main
  complexity is the Field control surface and safe pickup/catch behavior.

## System Architecture

```mermaid
block-beta
    columns 3

    field["Daisy Field hardware"]:1
    ui["Controls / LEDs / OLED"]:1
    dsp["field_mixer DSP"]:1

    hydra["Hydra IN L"]:1
    edge["Edge IN R"]:1
    out["OUT L + OUT R + headphones common"]:1

    field --> ui
    hydra --> dsp
    edge --> dsp
    ui --> dsp
    dsp --> out
```

## Signal Flow

```mermaid
flowchart LR
    H["Hydra IN L"] --> HT["Trim"] --> HD["DC block"] --> HH["HPF"] --> HC["Tone"] --> HG["Input gate"] --> HL["Level"]
    E["Edge IN R"] --> ET["Trim"] --> ED["DC block"] --> EH["HPF"] --> EC["Tone"] --> EG["Input gate"] --> EL["Level + ducking"]
    HL --> M["Mix bus"]
    EL --> M
    HL --> DS["Delay send bus"]
    EL --> DS
    DS --> D["Mono delay"]
    D --> DT["Delay tone"]
    DT --> DR["Delay return"]
    DR --> M
    M --> S["Optional soft saturation"]
    S --> L["Limiter / ceiling"]
    L --> O["OUT L, OUT R, headphones"]
```

## Control Flow

```mermaid
flowchart TD
    Boot["Boot defaults"] --> Main["Main loop"]
    Main --> Poll["Process Field controls"]
    Poll --> Bank{"Held switch?"}
    Bank -->|"none"| MainBank["K1-K8 main bank"]
    Bank -->|"SW1"| AltBank["K1-K8 alt bank"]
    Bank -->|"SW2"| UtilBank["K1-K8 utility layer"]
    Poll --> Keys["Handle A/B key actions"]
    MainBank --> Pickup["Pickup/catch"]
    AltBank --> Pickup
    UtilBank --> Pickup
    Pickup --> Display["OLED overview or edit zoom"]
    Keys --> Display
    Display --> LEDs["Update knob/key/switch LEDs"]
    LEDs --> Main
```

## Build

From this directory:

```sh
make
```

## Validation Status

This project may be marked hardware-validated only after:

- `make` succeeds
- QAE validation succeeds
- `make program` succeeds through ST-Link, if flashing is requested
- the manual checklist in `CONTROLS.md` is completed and dated

Until then, describe the result as `build-verified only - hardware validation
not performed`.
