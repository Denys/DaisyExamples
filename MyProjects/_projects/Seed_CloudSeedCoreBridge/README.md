# Seed_CloudSeedCoreBridge

Bridge project to prepare **CloudSeedCore** for Daisy hardware in a **controls-independent** way.

## What is included

- A normalized `ControlModel` (`0..1`) that is hardware agnostic.
- A Daisy Seed ADC mapping layer that writes into `ControlModel`.
- A Field mapping plan (`FieldMappingLayout`) so the same model can be reused on Daisy Field.
- A fallback DSP adapter (`ReverbSc`) so the project compiles and runs even before CloudSeedCore is linked.
- Rack-style per-class interactive enums (`ParamIds`, `InputIds`, `OutputIds`, `LightIds`) in `CloudSeedInteractiveParameters.h`.

## Why this structure

CloudSeedCore integration can be done once in an engine adapter, while each hardware target only needs a mapping layer.

```
Hardware (Seed/Field/MIDI)
          -> ControlModel (normalized)
          -> CloudSeed parameter map
          -> Audio engine
```

## CloudSeedCore import status

The environment used to produce this change could not reach GitHub (`403` tunnel/connect restriction), so the external repository could not be cloned automatically.

When network access is available, import CloudSeedCore manually:

```bash
cd /workspace/DaisyExamples
mkdir -p third_party
git clone https://github.com/GhostNoteAudio/CloudSeedCore third_party/CloudSeedCore
```

Then:

1. Add the CloudSeedCore source files to `CPP_SOURCES` in `Makefile`.
2. Add include paths for `third_party/CloudSeedCore`.
3. Set `CPPFLAGS += -DUSE_CLOUDSEEDCORE=1`.
4. Replace fallback adapter internals with concrete CloudSeedCore calls.

## Field hardware mapping (planned)

Recommended Field knob map:

- K1: Mix
- K2: Size
- K3: Decay
- K4: Diffusion
- K5: Pre-delay
- K6: Damping
- K7: Modulation amount
- K8: Modulation rate

These directly map to the same `ControlModel` used by Seed.


<<<<<<< ours
<<<<<<< ours
<<<<<<< ours
Detailed architecture schematic: see `CLOUDSEED_BLOCK_DIAGRAM.md` for signal flow, expandable DSP primitives/classes, and control ownership.
=======
Detailed architecture schematics: see `CLOUDSEED_BLOCK_DIAGRAM.md` (Markdown) and `CLOUDSEED_BLOCK_DIAGRAM.html` (interactive HTML) for signal flow, expandable DSP primitives/classes, and control ownership.

BKS-based parameter grouping reference: `CLOUDSEED_PARAMETER_BREAKDOWN_BKS.md` (derived from `bkshepherd/DaisySeedProjects` `Parameter2`).
>>>>>>> theirs
=======
Detailed architecture schematics: see `CLOUDSEED_BLOCK_DIAGRAM.md` (Markdown) and `CLOUDSEED_BLOCK_DIAGRAM.html` (interactive HTML) for signal flow, expandable DSP primitives/classes, and control ownership.

BKS-based parameter grouping reference: `CLOUDSEED_PARAMETER_BREAKDOWN_BKS.md` (derived from `bkshepherd/DaisySeedProjects` `Parameter2`).
>>>>>>> theirs
=======
Detailed architecture schematics: see `CLOUDSEED_BLOCK_DIAGRAM.md` (Markdown) and `CLOUDSEED_BLOCK_DIAGRAM.html` (interactive HTML) for signal flow, expandable DSP primitives/classes, and control ownership.

BKS-based parameter grouping reference: `CLOUDSEED_PARAMETER_BREAKDOWN_BKS.md` (derived from `bkshepherd/DaisySeedProjects` `Parameter2`).
>>>>>>> theirs

## Interactive parameter organization

Interactive parameters are now organized per class (similar to your Plaits-style enum layout):

- `CloudSeedGlobal`
- `CloudSeedTap`
- `CloudSeedEarlyDiffusion`
- `CloudSeedLateReverb`
- `CloudSeedEq`
- `CloudSeedSeeds`
- `CloudSeedPerformance` (8-knob Seed panel)

Each class exposes:

- `enum ParamIds`
- `enum InputIds`
- `enum OutputIds`
- `enum LightIds`
