# Field_MFOS_NoiseToaster Reference Diagrams Design

## Goal

Update the `Field_MFOS_NoiseToaster` documentation so it is more useful as both a project reference and an analog-faithfulness audit:

- patch the reusable generation prompt with better MIDI and OLED rules
- redraw the original MFOS Noise Toaster Figure 4-2 in Mermaid as the analog reference style
- redraw the current Daisy Field firmware as a firmware-accurate Mermaid block diagram in the same visual language
- add a target Mermaid block diagram that shows the highest-value analog-faithful improvement path without claiming that those changes already exist

## Sources Of Truth

Current firmware source of truth:

- `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`

Analog reference source of truth:

- `MyProjects/_projects/Field_MFOS_NoiseToaster/4-2_Noise_Toaster_Block_Diagram.png`
- `MyProjects/_projects/Field_MFOS_NoiseToaster/Make  Analog Synthesizers - Ray Wilson.pdf`

Relevant PDF sections used for the comparison:

- chapter 4 overview pages describing the Noise Toaster block diagram and normalized switching
- chapter 6 theory pages describing AREG, LFO, VCO, white noise, VCF, VCA, bypass, and output stage behavior

## Diagram Rules

The user approved three distinct diagram roles:

1. The Figure 4-2 redraw is the guiding-star reference and should mirror the analog architecture as closely as Mermaid allows.
2. The current project block diagram must be a 1:1 reflection of the currently implemented firmware, even where it departs from the analog design.
3. The post-improvement block diagram is a target architecture only. It exists to show how the updated Daisy Field project could move closer to Figure 4-2 while still being honest about board-level adaptation.

All project diagrams must clearly distinguish current behavior from target behavior.

## Scope

Documentation-only change:

- modify `GENERATION_PROMPT.md`
- modify `DIAGRAMS.md`
- update `README.md` with PDF-backed analog-faithfulness notes and revised improvement order
- update `CONTROLS.md` with the analog counterpart controls that are not currently exposed

No C++ firmware behavior changes are part of this pass.

## Prompt Update Design

### Conditional MIDI Rule

The Field and Pod prompt blocks should no longer claim that external MIDI note input is always required.

Instead:

- if the project is intended for pitched keyboard-style performance, require external MIDI keyboard note input
- if the project is not meant to be played from piano-style keys, MIDI is optional and omitted by default unless explicitly requested

### Field OLED Rule

The Daisy Field prompt should explicitly ask for:

- an idle OLED overview showing the most important live parameters when controls are not moving
- a temporary zoomed parameter view when a control is actively being changed

This rule should appear in both implementation requirements and documentation requirements.

## Analog-Faithfulness Interpretation

The PDF confirms several important structural facts about the original Noise Toaster:

- it is centered on manual gate / repeat behavior rather than a piano-key note layout
- the AREG is a first-class performance control with live attack and release controls
- the VCO has independent frequency, LFO mod depth, AR mod depth, and sync behavior
- the VCF has its own modulation source selector and modulation depth
- the LFO has three selectable wave shapes
- the VCA is AREG-driven and can be bypassed
- the output stage includes output level and internal amp / line-out switching

These facts should drive the revised improvement ranking.

## Revised Improvement Priority

The revised "most analog-faithful improvement per unit of work" order should favor features that restore the original control architecture with relatively contained firmware changes:

1. expose AREG attack and release as live controls
2. add Repeat / Manual behavior and a manual gate action
3. replace the fixed sine LFO with selectable square / differentiated square / integrated square behavior
4. add a real VCF modulation source selector and dedicated modulation depth behavior
5. add VCA bypass and move amplitude behavior closer to AREG-driven dynamics
6. add VCO sync behavior
7. replace the current continuous osc/noise crossfade with a more analog-like input selection model if strict faithfulness is desired

## Verification

This task is documentation-focused, so verification should consist of:

- reading the changed Markdown back
- checking that Mermaid blocks are present and labeled correctly
- running `git diff --check` on the changed docs

No firmware build is needed unless a code file changes unexpectedly.
