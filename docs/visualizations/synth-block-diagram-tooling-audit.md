# Synth Block Diagram Tooling Audit

Date: 2026-06-07

## Executive Summary

The Firecrawl report is directionally right that semantic, spec-backed diagrams
are the right path. The correction is that GoJS should not be the only default:
for this repo's static synth explanations, a small source-backed SVG/HTML
generator is a better first tool because it avoids GoJS license friction,
browser rendering brittleness, and auto-layout spaghetti.

Personal opinion: use the new `synth-block-diagram` skill for first-pass static
synth/control diagrams. Use GoJS when interaction is needed. Use Rete.js only
for executable DVPE-style patch editors.

## Current June BD Fidelity

The current June dashboard block diagram mostly reflects the real firmware:

- Audio chain is correct: pitch/glide -> oscillator/sub/noise -> source sum ->
  envelope/velocity amp -> SVF low-pass -> tanh drive -> level -> stereo out.
- Control grouping is mostly correct: main/alt knobs, A-row mode selectors,
  B-row controls/performance mode, hidden SW2+K8 level, OLED/LED feedback.
- A7 Pitch LFO is correctly better represented as a note in the pitch/frequency
  block than as a long backward wire, because the firmware applies it in
  `AudioCallback` frequency math.
- ADSR and LFO should stay as short local modulation stubs. Long shared
  modulation buses were the main readability failure and were removed.

Remaining interpretation notes:

- UI feedback is intentionally simplified; actual OLED/LED rendering covers
  more than the one visible feedback path.
- `K8 Alt Sub` affects sub level in both `ApplyVoiceSetup` and source mixing,
  so diagrams should label it as sub amount rather than pretending there is
  only one gain point.
- SW2 has two visible roles in June: rising edge toggles B-row
  Controls/Performance mode, while holding SW2 exposes hidden K8 level trim.

The screenshot also showed a concrete rendering bug: the `Pitch + gate` block
was too short after the A7 Pitch LFO note was added. The dashboard HTML was
updated to make that block taller and shorten the note.

## Tool Evaluation

| Tool / approach | Keep? | Best use | Limitation |
|---|---|---|---|
| Hand-routed HTML/SVG in dashboard | Yes | Final dashboard presentation | Hard to regenerate and validate unless backed by a spec |
| New `synth-block-diagram` skill + `synth_bd.py` | Yes | Source-backed static synth/control diagrams | New, intentionally small; not interactive |
| Local GoJS framework | Yes | Interactive/spec-backed engineering diagrams | Current schema is generic/electrical; production use needs license decision |
| Mermaid + FigJam | Yes | Quick editable review diagrams | Auto-layout loses port/control intent on dense synth diagrams |
| Graphify / knowledge graphs | Hold | Concept exploration and code/topic maps | Not a precision signal-flow geometry tool |
| Rete.js / React Flow | Later | Executable DVPE-style patch editor | Larger app/build task; unnecessary for static documentation |
| MCP server | Later | Shared diagram generator callable by many agents/tools | Not needed for the current local repo workflow |

## Created Tool

Installed skill:

```text
C:\Users\denko\.codex\skills\synth-block-diagram\SKILL.md
```

Renderer:

```text
C:\Users\denko\.codex\skills\synth-block-diagram\scripts\synth_bd.py
```

The renderer validates:

- required spec fields
- duplicate/missing block IDs
- unknown link endpoints
- block bounds
- estimated text clipping
- excessive horizontal modulation routes
- missing source trace references

It renders:

- standalone SVG
- standalone HTML
- optional PNG when `cairosvg` is available

## April Warm-Up Output

Generated from `Field_Template_April.cpp` and `README.md`:

```text
docs/visualizations/field-template-april/field-template-april-synth-bd.json
docs/visualizations/field-template-april/field-template-april-synth-bd.html
docs/visualizations/field-template-april/field-template-april-synth-bd.svg
docs/visualizations/field-template-april/field-template-april-synth-bd.png
docs/visualizations/field-template-april/field-template-april-synth-bd.verification.json
```

Validation result:

- `21` blocks
- `23` links
- `9` audio links
- `9` control links
- `3` modulation links
- `2` feedback links
- no validation issues

First iteration caught likely text clipping in four blocks. The spec was
updated, then the output was regenerated and visually inspected as PNG.

## MCP Decision

Do not build a live MCP server yet. The current need is deterministic local
generation from repo source into checked-in documentation artifacts. A skill
plus script is enough and easier to validate. Reconsider MCP when the generator
must be exposed to multiple clients, a dashboard app, or external automation
without direct filesystem/script access.
