# Project Portfolio

This file is a strategic classification of the repo portfolio, not a promise
that every listed project is currently verified. For any specific project,
confirm details in local docs and source.

## Foundational Workspaces

| Project | Bucket | Notes |
|---|---|---|
| `DaisyHost/` | Foundational Workspace, Active Iterating | Current host-side strategic workspace; pre-flash Patch host and future training/render bridge. |
| `DaisyDAFX/` | Foundational Workspace, Active Verified | Canonical first-party DSP library workspace; prefer over legacy DAFX copy. |
| `pedal/` | Foundational Workspace, Active Verified | New first-party board family workspace with documented examples and reference pipeline. |
| `DAISY_QAE/` | Foundational Workspace | Durable engineering standards and validation hub. |

## Active And Recently Important Custom Projects

These are the current high-confidence entries from `LATEST_PROJECTS.md`.

| Project | Bucket | Notes |
|---|---|---|
| `field/CloudSeed/` | Active Verified | New Field-first CloudSeed example using imported engine core; current memory headroom is tight but build verified. |
| `field/GranularSynth/` | Active Verified | New Field granular synth voice; build verified with required DaisySP LGPL path. |
| `MyProjects/_projects/Seed_CloudSeedCoreBridge/` | Active Iterating | Bridge/scaffolding state for future CloudSeed integration; useful but not the final canonical solution. |
| `MyProjects/_projects/Pod_MultiFX_Chain/` | Working Reusable, Active Verified | Strong Pod reference project with documented validation and page/control patterns. |
| `MyProjects/_projects/multi_fx_synth_pod/` | Working Reusable | Controls doc is newer than README; reuse with caution and prefer local docs. |
| `MyProjects/_projects/Pod_Harmonizer/` | Working Reusable | Intermediate harmonizer effect with local control docs. |
| `MyProjects/_projects/Seed_MIDI_Grainlet/` | Working Reusable | Seed + Teensy workflow reference with companion setup doc. |

## Often-Used Or Likely-To-Be-Reused References

| Project | Bucket | Why it matters |
|---|---|---|
| `MyProjects/_projects/Field_Template_April/` | Working Reusable | Field template/reference for new Field projects. |
| `MyProjects/_projects/Field_Template_Std/` and `_Std_2/` | Working Reusable | Standardized Field scaffolding references. |
| `MyProjects/_projects/Pod_OledEncoder_test/` | Working Reusable | Useful for OLED/encoder interaction patterns. |
| `MyProjects/_projects/Field_MI_Plaits/` | Working Reusable | Reference for Mutable-style engine integration on Field. |
| `MyProjects/_projects/Field_MI_Rings/` | Working Reusable | Resonator-style reference project. |
| `MyProjects/_projects/PlaitsPatchInit/` | Working Reusable | Relevant for `patch.init()` and future multi-board / VCV thinking. |
| `MyProjects/_projects/Pod_MarkovAIDrummer/` | Concept / Portfolio | Early bridge toward book-inspired Markov work. |
| `MyProjects/_projects/Pod_OLED_EuclideanDrums/` | Concept / Portfolio | Useful UI/control reference even if not current. |

## Field Portfolio Families

The `Field_*` portfolio is broad and heterogeneous. Treat it as a design pool,
not a uniformly verified product line.

Sub-families visible today include:

- additive synth variants
  - `Field_AdditiveSynth`
  - `Field_AdditiveSynth_0330`
  - `Field_AdditiveSynth_2`
  - `Field_AdditiveSynth_Copy`
  - `Field_AdditiveSynth_Meta`
- drum and rhythm systems
  - `Field_DrumLab`
  - `Field_DrumMachinePro`
  - `Field_EuclideanRhythmist`
  - `Field_GlitchDrumPerformer`
  - `Field_RetroStepSequencer`
  - `FieldOpus_DrumMachinePro`
- string, modal, and physical-model directions
  - `field_string_machine`
  - `field_string_machine_poly`
  - `Field_ModalBells`
  - `Field_stringvoice_overdrive_reverb`
- wavetable and drone directions
  - `Field_WavetableDroneLab`
  - `Field_WavetableSynth`
  - `field_wavetable_morph_synth`
- other musical concepts
  - `Field_AmbientGarden`
  - `Field_Pollen`
  - `Field_MFOS_NoiseToaster`
  - `Field_SynthFxWorkstation`
  - `FieldArpeggiator`

Strategic read:

- strong concept inventory
- uneven verification confidence
- useful source of ideas and UI/control patterns

## Pod And Seed Portfolio Families

Pod-heavy directions include:

- multi-FX and performance processors
  - `Pod_MultiFX_Chain`
  - `multi_fx_synth_pod`
  - `Pod_SpectralMutator`
  - `Pod_SynthFxWorkstation`
  - `POD_EDGE_mono_DSP`
- drum and sequencing directions
  - `Pod_DrumMachinePro`
  - `Pod_OLED_EuclideanDrums`
  - `Pomodoro_Pod`
- harmonizer / resonator / physical-model directions
  - `Pod_Harmonizer`
  - `pod_nebula_resonator`
  - `Pod_PhysicalModel`
- AI / algorithmic direction
  - `Pod_MarkovAIDrummer`

Seed-heavy directions include:

- `Seed_CloudSeedCoreBridge`
- `Seed_MIDI_Grainlet`
- `ex_tube_seed`

## Experiments And Scratch

`MyProjects/_experiments/` is the experiment bucket. Current examples include:

- `ExampleProject`
- `ExampleProjec_openrouter`
- `MyExampleProject`
- `MyKeyboardTest`

These are useful for targeted probing, not for strategic assumptions.

## Transitional / Legacy / Low-Confidence Entries

These should not be treated as current without explicit re-checking:

- `MyProjects/DAFX_2_Daisy_lib/`
- `_DrumMachine_Project`
- `New folder`
- directories with no local docs and unclear recent validation

## Strategic Reading Of The Portfolio

- The repo has a small number of foundational workspaces.
- It has a medium-sized set of active verified or reusable projects.
- It has a large long tail of concept-heavy firmware projects that are valuable
  for idea mining but should not be assumed current.
- DaisyBrain should preserve this distinction instead of flattening everything
  into a single "all projects are equal" memory view.
