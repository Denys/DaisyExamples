# Board-Specific Generation Prompts Design

## Goal

Turn `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md` from a single project-specific prompt into a simple copy-paste prompt kit with two reusable sections:

- Daisy Field specific
- Daisy Pod specific

## Scope

Documentation-only change:

- Rewrite the existing `GENERATION_PROMPT.md`
- Preserve the core quality bar from the current version
- Replace project-specific MFOS Noise Toaster content with placeholders and board-specific hardware vocabulary

No C++ source or runtime behavior changes are included.

## User Need

The file should be easy to open and copy directly into an AI chat without further restructuring. That means:

- one file
- two self-contained prompt blocks
- minimal surrounding explanation
- board-appropriate control vocabulary already baked in

## Recommended Structure

### File shape

`GENERATION_PROMPT.md` should contain:

1. Short intro explaining how to use the file
2. Short placeholder replacement checklist
3. `Daisy Field Prompt` section with one full prompt block
4. `Daisy Pod Prompt` section with one full prompt block

### Daisy Field prompt

The Field prompt should assume:

- `K1-K8`
- `A1-A8`
- `B1-B8`
- `SW1/SW2`
- optional OLED, serial log, MIDI

It should instruct the model to:

- generate code and docs together
- keep the audio callback real-time safe
- keep control/UI processing outside the callback
- generate `README.md`, `CONTROLS.md`, and `DIAGRAMS.md`
- avoid inventing controls or routings

### Daisy Pod prompt

The Pod prompt should assume:

- `Knob 1`
- `Knob 2`
- `Encoder turn`
- `Encoder press`
- `Button 1`
- `Button 2`
- `LED1`
- `LED2`
- optional serial log, MIDI

It should instruct the model to:

- generate code and docs together
- document page model and combo actions if used
- avoid inventing displays or controls not present on Daisy Pod

## Quality Bar

- ASCII Markdown only
- self-contained prompt blocks
- direct copy-paste friendly
- placeholders instead of MFOS-specific content
- preserve strong implementation and documentation guardrails

## Verification

- `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`
- read back the rewritten prompt file
- confirm both board sections are present and board-specific
