# Current State (2026-04)

## Top-Level Strategic Picture

The repo currently has four especially important first-party centers:

- `DaisyHost/`
- `DaisyDAFX/`
- `pedal/`
- `DAISY_QAE/`

The most strategically active one is `DaisyHost/`.

## Current Strategic Project: DaisyHost

Current role:

- virtual Daisy Patch plugin and standalone app
- shared-core architecture proving host + firmware alignment
- pre-flash testing environment
- future automation, training, and rendering bridge

Current maturity:

- good enough to function as a real first-party workspace
- still not yet at the training gate required for MetaController work

Current next architectural gate:

- canonical parameter contract
- direct parameter API
- deterministic reset and full state serialization
- effective-state readback
- parity tests
- headless rendering

That gate matters more than additional UI polish or multi-node expansion.

## Current Mature / Stable First-Party Areas

### `DaisyDAFX/`

- mature reusable DSP library workspace
- canonical home for DAFX-derived modules
- should be preferred over the legacy transitional copy

### `pedal/`

- new board-family workspace with verified examples and docs pipeline
- shows the repo is now capable of supporting durable non-upstream board families

### `DAISY_QAE/`

- stable source of local engineering doctrine
- should increasingly absorb durable cross-project guidance

## Current Active Firmware Frontier

Recent and notable:

- `field/CloudSeed/`
- `field/GranularSynth/`
- `MyProjects/_projects/Seed_CloudSeedCoreBridge/`
- `MyProjects/_projects/Pod_MultiFX_Chain/`

Interpretation:

- Field work is active and no longer just speculative.
- CloudSeed remains both a current implementation target and a bridge/migration
  topic.
- Pod projects continue to provide reusable control/UI patterns.

## Current Strategic Risks

- the repo has many concept-rich projects but only a smaller verified subset
- agents can overfit to one subtree if they ignore pinned workspace roots
- local docs are uneven across the long tail of `_projects`
- DaisyHost can attract feature sprawl before its parameter/render contract is stable

## Current Strategic Opportunities

- DaisyHost can become the single best place for pre-flash Daisy iteration
- DaisyDAFX can supply clean reusable DSP modules into both host and firmware work
- Pedal proves the repo can sustain first-party board ecosystems
- DaisyBrain can reduce repeated rediscovery if it remains curated and layered
