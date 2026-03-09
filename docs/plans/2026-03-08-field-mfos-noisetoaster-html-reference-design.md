# Field_MFOS_NoiseToaster HTML Reference Design

## Goal

Create a single self-contained HTML artifact for `Field_MFOS_NoiseToaster` that does two jobs:

1. present a firmware-accurate block-diagram equivalent to `4-2_Noise_Toaster_Block_Diagram`
2. visually map Daisy Field controls onto the `4-8_NT_controls_front_panel` reference panel

The same artifact should also include an interactive mode that can later support control-interface brainstorming for future firmware revisions.

## Approved Direction

Use one HTML file with two modes built from one shared mapping model:

- `Reference Mode`
  A documentation-focused layout with:
  - a firmware block diagram based strictly on `Field_MFOS_NoiseToaster.cpp`
  - a front-panel overlay on top of `4-8_NT_controls_front_panel.png`
  - a compact legend that shows Daisy Field control -> Noise Toaster function mappings

- `Interactive Brainstorm Mode`
  A second view in the same page that reuses the same blocks and control definitions, but allows the user to click items and inspect:
  - current firmware mapping
  - fixed internal settings
  - analog-original controls that are not implemented
  - future remap opportunities

## Content Rules

The HTML should reflect the current firmware implementation, not the original analog panel as an aspirational target.

The artifact must visibly include:

- `K1 -> VCO Frequency / Coarse Tune`
- `K2 -> VCO LFO Depth`
- `K3 -> VCO AREG Depth`
- `K4 -> VCF Cutoff`
- `K5 -> VCF Resonance`
- `K6 -> VCF Mod Depth`
- `K7 -> AREG Attack`
- `K8 -> AREG Release`
- `B1 -> VCO Wave Cycle`
- `B2 -> LFO Wave Cycle`
- `B3 -> VCF Mod Source`
- `B4 -> Repeat / Manual`
- `B5 -> VCA Bypass`
- `SW1 -> Manual Gate`
- `SW2 -> Panic`
- `A1..A8 -> Field-only note selection / trigger adaptation`

The artifact must also clearly mark fixed internal firmware settings:

- `LFO Rate = 2.2 Hz`
- `Noise Blend = 18%`
- `Output Level = 72%`

## Visual Direction

Use a single-page technical-reference aesthetic:

- vintage synth service-manual / bench-instrument mood
- warm paper and dark chassis tones rather than generic web-app styling
- strong diagram readability first
- subtle animated highlights for hover and selection
- distinct color coding for:
  - audio path
  - modulation path
  - Daisy control mappings
  - fixed internal / not-implemented items

## Layout

The page should be organized into these sections:

1. header / mode toggle
2. firmware block diagram
3. front-panel mapping overlay
4. mapping legend
5. interactive inspector / brainstorm notes

## Data Model

Use one in-file JavaScript data model for:

- firmware blocks
- connections
- front-panel hotspot definitions
- Daisy Field mapping labels
- status metadata (`mapped`, `field-only`, `fixed internal`, `missing analog`)
- brainstorm notes

This keeps the documentation and interactive mode synchronized.

## Files

- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

## Verification

Add a small regression check that confirms the HTML file exists and contains the key required sections and representative mappings.
