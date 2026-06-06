# Field Template June Dashboard Report

Generated: 2026-06-06

## Executive Summary

This dashboard documents `Field_Template_June`, a separate Daisy Field project
created from `Field_Template_April` with a controls-optimized runtime. The new
project replaces April's near-value pickup/catch with an until-touched movement
gate, caps OLED redraw to `50 ms`, caps LED transmit to `16 ms`, applies voice
setup only when dirty, bounds MIDI processing to `16` events per main-loop tick,
and adds a startup control-prime phase before audio starts.

## Conclusions

`Field_Template_June` is the stronger reference for future Field templates. It
keeps April's useful small synth surface while adopting the more efficient
runtime scheduling pattern proven by `Field_delay_bundle`. The implementation is
software-verified by build and QAE; hardware validation remains pending until a
Daisy Field is connected.

## Personal Opinion

The most important design choice is the new control semantics. Until-touched
behavior maps better to the user's mental model than pickup/catch for a template:
a bank or modifier switch protects stored values, and editing resumes as soon as
the knob is intentionally moved in the active context.

## Artifact

- Dashboard: `index.html`
- Project: `../../../MyProjects/_projects/Field_Template_June/`
- Hardware test plan: `../../../MyProjects/_projects/Field_Template_June/HARDWARE_TESTPLAN.md`
- Hardware test runner: `../../../MyProjects/_projects/Field_Template_June/run_hardware_testplan.ps1`

## Dashboard Pages

- `#overview`: implementation summary and current status.
- `#architecture`: Mermaid architecture and GoJS interactive graph.
- `#timing`: timestamped runtime lanes and timing sequence.
- `#responsiveness`: until-touched state machine and April-vs-June behavior.
- `#cpu-memory`: Field template memory comparison.
- `#io`: visual Field control/OLED schematic and control-display flow.
- `#parameters`: main, alt, and hidden parameter mapping.
- `#build`: build, validation, and ST-Link flash commands.
- `#tests`: build/QAE/runner/hardware status.
- `#logs`: command evidence and source line references.

## Current Verification

| Surface | Check | Result |
|---|---|---|
| `Field_Template_June` firmware | `make` | PASS |
| `Field_Template_June` QAE | `py -3 ../../../DAISY_QAE/validate_daisy_code.py .` with UTF-8 output | PASS, `0 error(s), 0 warning(s)` |
| Hardware test runner | `.\run_hardware_testplan.ps1 -SkipBuild -NonInteractive` | PASS |
| Hardware flash/run | `make program` | Pending hardware |

## Build Footprint

| Region | Used | Capacity | Usage |
|---|---:|---:|---:|
| FLASH | `115688 B` | `128 KB` | `88.26%` |
| SRAM | `52920 B` | `512 KB` | `10.09%` |
| SDRAM | `0 B` | `64 MB` | `0.00%` |

## Source Evidence

| Claim | Evidence |
|---|---|
| June uses movement threshold `0.012`, OLED cap `50 ms`, LED cap `16 ms`, MIDI cap `16` | `Field_Template_June.cpp:18-26` |
| June touch gate compares raw value against entry anchor | `Field_Template_June.cpp:229` |
| June records new anchors on bank switch | `Field_Template_June.cpp:341-348` |
| June hidden level uses its own touch gate | `Field_Template_June.cpp:513` |
| June bank knobs use until-touched capture | `Field_Template_June.cpp:534` |
| June visual outputs are deferred | `Field_Template_June.cpp:783-797` |
| June startup display and control prime are implemented | `Field_Template_June.cpp:896-924` |
| April used pickup/catch near stored value | `Field_Template_April.cpp:18`, `Field_Template_April.cpp:433-460` |
| Field delay reference uses the same movement and timing constants | `FieldDelayFieldApp.h:33-38`, `FieldDelayFieldApp.h:332-345`, `FieldDelayFieldApp.h:736-741` |

## Notes

The dashboard is static and opens directly from disk. Mermaid and GoJS rendering
use CDN scripts, so live diagrams require network access. If the CDN is
unavailable, the written tables and project docs remain the source of truth.
