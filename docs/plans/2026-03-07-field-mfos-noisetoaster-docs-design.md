# Field_MFOS_NoiseToaster Documentation Design

## Goal

Bring `MyProjects/_projects/Field_MFOS_NoiseToaster` up to the same practical documentation level as the richer `Field_AdditiveSynth` project docs, while keeping the documentation accurate to the current firmware implementation.

## Scope

Documentation-only change:

- Expand `README.md` into the operator-facing overview.
- Add `CONTROLS.md` for detailed hardware mapping and runtime behavior.
- Add `DIAGRAMS.md` for architecture and signal/control flow.
- Add `GENERATION_PROMPT.md` with a reusable prompt that asks an AI to generate the code and the required documentation together.

No firmware behavior changes are included in this task.

## Source Of Truth

The documentation must match the current implementation in:

- `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- `MyProjects/_projects/Field_MFOS_NoiseToaster/Makefile`

Important implementation details to preserve:

- Mono synth voice with a single VCO, white noise mix, SVF low-pass filter, fixed VCA ADSR, fixed AR contour, and stereo mirrored output.
- `A1..A8` trigger notes `C3, D3, E3, F3, G3, A3, B3, C4`.
- `B1/B2/B3` select saw, square, triangle.
- `B4` and `SW1` both toggle hold.
- `B5` toggles AR destination between pitch and filter.
- `SW2` is panic: gate off and hold off.
- LFO depth affects pitch and filter together; it is not user-routable in the current code.
- The synth is monophonic with last-trigger behavior and no held-note stack.

## Deliverables

### README.md

Purpose:

- Give a fast overview of the synth and its architecture.
- Provide quick controls, startup/default behavior, build instructions, and links to the detailed docs.

### CONTROLS.md

Purpose:

- Document knobs, keys, switches, note map, OLED text, serial boot log, and runtime interaction behavior.
- Include concrete parameter behavior where the code makes it clear, such as tune range, LFO rate mapping, and resonance range.

### DIAGRAMS.md

Purpose:

- Provide Mermaid diagrams for:
  - Hardware/control block overview
  - Audio signal flow
  - Main-loop and event flow

### GENERATION_PROMPT.md

Purpose:

- Provide a copy-pastable prompt for generating a complete Daisy Field project plus all required accompanying docs.
- Require the generated docs to stay consistent with the code.

## Quality Bar

- Use ASCII Markdown only.
- Prefer concise operator-facing language over generic prose.
- Avoid claiming features that do not exist in the code.
- Keep the generated prompt reusable and explicit about required outputs.

## Verification

Verification for this task is documentation-focused:

- Check file creation and diffs.
- Run `git diff --check` to catch whitespace/patch issues.
- Do not claim a firmware build was revalidated, because the task does not change C++ and the environment is missing local libDaisy core files.
