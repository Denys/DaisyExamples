# Field Template April Synth Block Diagram

This folder contains a warm-up source-backed block diagram for
`MyProjects/_projects/Field_Template_April`.

Generated artifacts:

- `field-template-april-synth-bd.json` - editable diagram spec
- `field-template-april-synth-bd.html` - standalone browser page
- `field-template-april-synth-bd.svg` - vector diagram
- `field-template-april-synth-bd.png` - PNG inspection export
- `field-template-april-synth-bd.verification.json` - render/validation summary

Regenerate from the repo root:

```powershell
py -3 C:\Users\denko\.codex\skills\synth-block-diagram\scripts\synth_bd.py validate --spec docs\visualizations\field-template-april\field-template-april-synth-bd.json
py -3 C:\Users\denko\.codex\skills\synth-block-diagram\scripts\synth_bd.py render --spec docs\visualizations\field-template-april\field-template-april-synth-bd.json --out-dir docs\visualizations\field-template-april --name field-template-april-synth-bd
```

For PNG export, install or expose `cairosvg`, then add `--png` to the render
command.

The diagram is based on `Field_Template_April.cpp` and `README.md`: external
MIDI supplies note/gate; A/B key rows are controls; SW1 is hold-alt; SW2+K8 is
hidden level; audio flows oscillator/sub/noise -> sum -> envelope/velocity amp
-> SVF low-pass -> tanh drive -> level -> stereo output.
