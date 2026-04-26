# DaisyHost Workstream Tracker

Last updated: 2026-04-26

This file tracks the next forward-looking DaisyHost portfolio after the current
`WS1` through `WS7` milestone set.

Current baseline:

- `WS1` through `WS7` planned milestone scope is complete enough to freeze in
  this checkout.
- Latest full host gate:
  `cmd /c build_host.cmd` passed on 2026-04-26 and `ctest` passed `202/202`.
- Daisy Field board-support shell, host-side Field native controls, and Field
  extended host surface support are now implemented:
  `daisy_field` flows through Hub, session, standalone startup, render, and
  smoke paths while the rack remains frozen, including host-side Field
  outputs/switches/LEDs, startup-request launch planning, and selected-node
  Field surface render evidence. Field K5-K8 now avoid explicit K1-K4
  parameter targets, and the Field editor uses board-profile target metadata
  for the first board-generic cleanup pass. Sprint F3 also adds
  `field/MultiDelay` as the first Field firmware adapter; it is build-verified
  and ST-Link flash-verified, with manual functional hardware validation still
  pending. Adapter-pipeline v0 now generates a build/QAE-verified
  `field/MultiDelayGenerated` adapter from a checked-in spec and audits firmware
  portability without claiming arbitrary source translation.
- This tracker is mirrored in `PROJECT_TRACKER.md` so the active ledger and the
  forward portfolio stay readable in one place.
- The TF12 adoption slice is implemented as docs plus verification hardening:
  `DaisyHostCLI` is in the current build target list, CTest covers its core
  smoke commands, and the documented agent/CI smoke sequence passed directly.

## Portfolio Rules

- Product workstreams optimize operator-facing value and demo quality.
- Technical-foundation workstreams reduce future integration cost and keep the
  platform extensible.
- Daisy Field has landed as a board-support shell plus host-side Field native
  controls and host-side Field outputs/switches/LEDs through the existing board
  factory seam; the first `field/MultiDelay` firmware adapter now exists and
  has build plus ST-Link flash/verify evidence, and adapter-pipeline v0 can
  generate the same class of Field firmware adapter from a shared-core spec.
  Full manual Field hardware validation, generated-adapter flashing, broader
  firmware parity, arbitrary firmware import, real voltage-output measurement,
  Field-specific app ergonomics, mixed-board racks, and manual DAW/VST3
  validation remain separate future work. Field-specific follow-on scope is
  tracked in `FIELD_PROJECT_TRACKER.md`.
- Every implementation start still needs a per-iteration claim in
  `PROJECT_TRACKER.md`.

## Product Value Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Status |
|---|---|---|---|---|---|
| `WS8` | Rack UX productionization | Makes the visible 2-node rack feel shippable: clearer node context, stronger role labels, better selected-node feedback, and fewer operator mistakes. | Frozen `WS7` rack baseline | `TF8`, `TF9`, `TF12`, `WS9` | Planned |
| `WS9` | Richer live routing presets | Expands the rack past the current four audio-only presets without jumping to a freeform graph editor. | `WS7` rack runtime; `TF10` contract cleanup | `WS8`, `TF10`, `TF11`, `TF12` | Planned |
| `WS10` | External state / debug surface | Exposes the effective host state outside the processor for tooling, QA, diagnostics, and demos. | Existing snapshot model; clearer node-targeted event rules from `TF11` | `TF11`, `TF12`, `WS8` | Planned |
| `WS11` | Hub + scenario workflow upgrade | Turns Hub into a launch surface for curated rack setups, saved scenarios, and repeatable operator flows. | Stable rack UX from `WS8`; board-aware planning from `TF8` / `TF9` | `WS8`, `TF8`, `TF9`, `TF12` | Planned |
| `WS12` | DAW-facing polish | Improves host-facing ergonomics and validates real VST3 behavior after the rack baseline is frozen. | Stable rack UX from `WS8`; verification hardening from `TF12` | `WS8`, `TF12` | Planned |

## Technical Foundation Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Status |
|---|---|---|---|---|---|
| `TF8` | Daisy Field board support | Adds `daisy_field` through the board factory seam plus host-side Field native controls, host-side Field outputs/switches/LEDs, and first shared-core-to-Field firmware adapter generation so Field work can ship without reopening Patch-only architecture. | Green `WS7` freeze gate | `WS8`, `WS11`, `TF9`, `TF12` | Extended host surface + adapter pipeline v0 implemented |
| `TF9` | Board-generic editor surface | Removes remaining Patch-shaped assumptions from the editor and board rendering path. | Existing board seam; pairs naturally with `TF8` | `TF8`, `WS8`, `WS11` | First cleanup implemented |
| `TF10` | Routing contract generalization | Stabilizes route validation and internal graph rules so richer routing does not become a rewrite every sprint. | `WS7` rack/session/render baseline | `WS9`, `TF11`, `TF12` | Planned |
| `TF11` | Node-targeted event surface expansion | Broadens the node-scoped event model for live/render/debug tooling beyond the current first-pass contract. | `WS7` node-targeted runtime | `WS9`, `WS10`, `TF10` | Planned |
| `TF12` | Verification / build hardening | Keeps `build_host.cmd`, smoke coverage, and checkout verification boring and repeatable. | Current wrapper and smoke harness | All workstreams | Verification/adoption slice implemented |

## ASCII Parallelization View

```text
Time ------------------------------------------------------------------------>

Frozen baseline:
  [WS1-WS7 complete] ----> [Daisy Field extended host surface implemented]

Product track:
  [WS8 Rack UX] -----------------------> [WS11 Hub + Scenario]
  [WS9 Routing Presets] ---------------> [WS12 DAW Polish]
  [WS10 External State / Debug]

Technical track:
  [TF8 Daisy Field] -------------------> [TF9 Board-Generic Editor]
  [TF10 Routing Contract] -------------> [TF11 Node Events]
  [TF12 Verification Hardening] ------------------------------------------+
                                                                         |
Cross-links:                                                             |
  TF10 -> WS9                                                            |
  TF11 -> WS10                                                           |
  TF8  -> WS11                                                           |
  WS8  -> WS11, WS12                                                     |
  TF12 -> all parallel lanes --------------------------------------------+
```

## Recommended Parallel Start

Start these first if staffing exists:

- `WS8`
- `WS9`
- `TF9`
- `TF12`

Start these after the first joins settle:

- `WS10` once `TF11` stabilizes the event/readback shape
- `WS11` once `WS8` and Field hardware follow-on expectations stop moving
  quickly
- `WS12` once `WS8` and `TF12` are stable enough for DAW-facing validation
- `TF10` and `TF11` can start early if owned as bounded contract-first tasks

## Decision Summary

- The best product-side next move is not another hidden infrastructure pass; it
  is making the rack easier to use, route, inspect, and launch.
- The best foundation-side next move is not more Patch-only polish; it is
  board-generic UI cleanup, routing-contract hardening, verification
  resilience, and the deferred hardware-facing Field work after the host-side
  Field extended surface package.
