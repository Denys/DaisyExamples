# Pod MultiFX Page Controls Design

**Date:** 2026-04-14

## Goal

Refactor `Pod_MultiFX_Chain` so the encoder changes the selected effect page,
`SW1` toggles bypass for the selected effect, `SW2` toggles a global bypass for
the full chain, and each effect keeps its stored settings across page changes
using soft takeover.

## Scope

- Replace button-based page selection with encoder-based page selection
- Add global bypass without removing per-effect bypass
- Preserve per-effect parameter state across page changes
- Add soft takeover so page switches do not immediately jump to raw knob
  positions
- Update project documentation
- Update `DAISY_QAE` standards so banked/page-switched Seed and Field projects
  treat pickup/catch/soft takeover as the default rule

## Control Rules

### Encoder

- Encoder turn selects the current effect page
- Page order remains:
  - Overdrive
  - Delay
  - Reverb
  - WahWah

### Switches

- `SW1` toggles bypass for the selected effect page only
- `SW2` toggles global bypass for the full chain

### Knobs

- `Knob 1` and `Knob 2` always edit the currently selected effect page
- Each effect page owns its own stored knob values
- On page change, stored values stay active immediately
- Knobs must reattach via soft takeover before they can change the newly
  selected page

## State Model

The current implementation stores live DSP values in globals and writes the
selected page directly from raw knob positions. That is why page changes do not
preserve state cleanly.

The refactor should introduce explicit page-owned state:

- selected page index
- global bypass flag
- per-page bypass flags
- per-page normalized knob values
- per-page soft-takeover armed/captured flags

This state should be independent from the DSP modules themselves. The audio
callback should apply DSP parameters from stored page state every block, rather
than from whichever page is currently selected.

## Soft Takeover Rule

Soft takeover means:

- the selected page keeps using its saved values immediately after page entry
- the physical knob does not overwrite the stored value on entry
- the knob starts editing only after the physical position crosses the saved
  value within a small threshold

This is the same behavior already described elsewhere in the repo as
"pickup/catch."

## Audio Behavior

- Signal chain remains serial:
  - Overdrive -> Delay -> WahWah -> Reverb
- Global bypass returns dry mono input to stereo output
- Per-effect bypass still works when global bypass is off
- Inactive pages keep their values, so returning to a page restores its last
  settings exactly

## LED Behavior

- LED color tracks the selected effect page
- LED brightness reflects effective bypass state:
  - dim when global bypass is enabled
  - otherwise dim when the selected effect page is bypassed
  - full brightness when the selected effect is active and global bypass is off

## Implementation Notes

- Keep analog control polling in the audio callback per `DAISY_QAE` BUG-005
- Keep encoder and button processing in the main loop
- Extract page-control logic into a small Daisy-independent helper so it can be
  tested on the host with standard `g++`
- Prefer a minimal helper over a reusable framework for this project

## Verification Strategy

- Add a host-side C++ test for:
  - encoder page wrapping
  - soft takeover arming on page change
  - capture behavior when crossing stored values
  - per-effect value persistence
  - global bypass toggling
- Run the host test and confirm it fails before implementation
- Re-run the host test after implementation
- Run `DAISY_QAE` lint
- Run a clean firmware rebuild

