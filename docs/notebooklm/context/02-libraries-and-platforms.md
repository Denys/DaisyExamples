# Libraries And Platforms

## Standard Platform Libraries

### `libDaisy/`

- upstream board/platform layer
- owned as a submodule, not as a first-party local project
- edit only when the task explicitly requires platform-level changes

### `DaisySP/`

- upstream DSP library
- used heavily across examples and local projects
- local projects may require specific flags for LGPL modules such as `ReverbSc`

### `stmlib/`

- supporting utility/source tree used by some synthesis ports and imported code

### `third_party/`

- imported or mirrored engines and dependencies that support newer workspaces
- especially relevant for CloudSeed-related and future engine-integration work

## First-Party Library And Platform Workspaces

### `DaisyDAFX/`

Role:

- canonical first-party portable DSP library derived from DAFX work
- intended to sit beside `DaisySP` and `libDaisy`, not inside `MyProjects`

State:

- mature and actively maintained
- host-side CMake/CTest workflow
- reference docs pipeline exists
- latest known completed pass canonicalized naming and strengthened tests

Strategic value:

- reusable DSP building blocks for future firmware and host-side projects
- reference example for portable, hardware-independent Daisy-targeted DSP code

### `pedal/`

Role:

- first-party DaisyPedal board workspace
- examples for a GuitarPedal125b-oriented control surface

State:

- Phase 1 is implemented and build-verified
- includes reference docs pipeline similar to DaisyDAFX

Strategic value:

- establishes a pattern for "new board family as first-party workspace"
- relevant precedent for future board expansion outside the upstream examples

### `DAISY_QAE/`

Role:

- engineering standards and validation hub

State:

- durable repo doctrine
- includes standards, validators, bug logs, and scaffolding guidance

Strategic value:

- the place where local Daisy workflow decisions should become durable
- preferred home for cross-project control, validation, and architecture rules

### `DaisyHost/`

Role:

- host-side JUCE/CMake virtual Patch plugin and standalone app

State:

- current first-party strategic workspace
- version `0.2.0`
- currently centered on a shared-core `MultiDelay` test algorithm

Strategic value:

- pre-flash testing
- future deterministic rendering and dataset generation
- future SuperKnob, MetaController, Markov, and Neural FX host path

## Transitional And Legacy Library Areas

### `MyProjects/DAFX_2_Daisy_lib/`

- deprecated transitional copy of older DAFX work
- use `DaisyDAFX/` as the canonical library root when the two differ
- keep only for compatibility or historical reference unless explicitly asked

## Practical Rule

When an agent needs reusable DSP or new board/platform patterns:

- prefer `DaisyDAFX/` over ad hoc DSP copies
- prefer `pedal/` and `DaisyHost/` as models for new first-party workspaces
- prefer `DAISY_QAE/` for durable engineering doctrine
