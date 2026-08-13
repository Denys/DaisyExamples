# Field_delay_bundle Synth Block Diagram

This folder contains the source-backed block diagram generated with the local
`synth-block-diagram` skill.

## Outputs

- `field-delay-bundle-bd.json` - editable diagram spec with trace notes
- `field-delay-bundle-bd.svg` - static vector render
- `field-delay-bundle-bd.png` - raster render for quick inspection
- `field-delay-bundle-bd.html` - scrollable HTML page with embedded SVG and
  source trace list
- `field-delay-bundle-bd.verification.json` - renderer verification summary

## Source Basis

The bundle firmware entrypoint is only a wrapper. The actual behavior is split
between:

- `MyProjects/_projects/Field_delay_shared/FieldDelayFieldApp.h`
- `DaisyHost/include/daisyhost/DaisyDelayFxCore.h`
- `DaisyHost/src/DaisyDelayFxCore.cpp`
- `MyProjects/_projects/Field_delay_bundle/CONTROLS.md`

The diagram shows one active `DaisyDelayFxCore::Process` path, with A1-A4
selecting exactly one of four algorithms:

- A1 Tape `[multifx]`
- A2 Tank `[reverb]`
- A3 Texture `[FunBox]`
- A4 Long `[sdram]`

## Verification

Regenerate from the repo root:

```powershell
py -3 C:\Users\denko\.codex\skills\synth-block-diagram\scripts\synth_bd.py validate --spec docs\visualizations\delay-algorithm-structure\synth-block-diagram\field-delay-bundle-bd.json
$env:PYTHONPATH = Join-Path $env:TEMP 'codex-cairosvg'
py -3 C:\Users\denko\.codex\skills\synth-block-diagram\scripts\synth_bd.py render --spec docs\visualizations\delay-algorithm-structure\synth-block-diagram\field-delay-bundle-bd.json --out-dir docs\visualizations\delay-algorithm-structure\synth-block-diagram --name field-delay-bundle-bd --png
```

Current validation result: pass. Render summary: 22 blocks, 16 links, 6 audio
links, 6 control links, 4 UI/state feedback links.

## Evaluation

The current block diagram is good for first-pass comprehension because it keeps
the audible signal path simple:

`Audio In -> Input + Synth Sum -> DaisyDelayFxCore::Process -> Selected Algorithm -> Dry/Wet -> Audio Out`

The first rendering pass drew all four algorithm branches as audio routes. That
was technically source-backed but visually too heavy. The current version treats
A1-A4 as selectable implementation cards and shows only the one active execution
path, which better matches the `switch(source_)` structure in the DSP core.

Recommended next improvements:

- Add one separate detail sheet per algorithm if low-level delay-line feedback
  topology matters. The bundle overview should not carry every tap and feedback
  path.
- Add a small timing lane below the diagram if this becomes part of a dashboard:
  48-frame audio callback, 1 ms main loop delay, 50 ms OLED update, 16 ms LED
  update.
- Add an explicit "source snapshots do not store audio delay buffers" note if
  users confuse per-algorithm parameter snapshots with delay-line memory.
