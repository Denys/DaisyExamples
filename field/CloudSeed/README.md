# CloudSeed

## Author

OpenAI Codex

## Description

Field port of Ghost Note Audio's `CloudSeedCore` reverb algorithm.

This example uses the real `third_party/CloudSeedCore` engine, not a fallback
reverb shim. It exposes an 8-knob performance layer on Daisy Field and keeps
the heavier CloudSeed parameters behind a compact OLED status page.

## Controls

| Control | Function |
| --- | --- |
| Knob 1 | Dry/wet mix |
| Knob 2 | Size |
| Knob 3 | Decay |
| Knob 4 | Diffusion |
| Knob 5 | Pre-delay |
| Knob 6 | Damping |
| Knob 7 | Modulation amount |
| Knob 8 | Modulation rate |
| SW 1 / SW 2 | Change OLED page |
| Key A1 | Toggle bypass |
| Key A2 | Clear tails |
| Key A3 | Toggle early diffusion |
| Key A4 | Toggle late diffusion |
| Key A5 | Toggle low cut |
| Key A6 | Toggle high cut |
| Key A7 | Randomize delay/diffusion seeds |
| Key A8 | Toggle interpolation |

## Notes

- `CloudSeedCore` is imported into `third_party/CloudSeedCore`.
- The Field example uses `ProgramDarkPlate` as its baseline voicing and remaps
  the 8 knobs into a performance-oriented macro layer.
- The bottom-row key LEDs indicate the active OLED page.
