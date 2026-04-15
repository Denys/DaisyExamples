# Pod MultiFX Reattach and Slew Design

**Date:** 2026-04-15

## Goal

Replace hard soft-takeover behavior in `Pod_MultiFX_Chain` with page-owned
position memory, first-move reattachment, and a short control slew that avoids
abrupt jumps when a knob becomes active again.

## Scope

- Remove soft-takeover wording and behavior from `Pod_MultiFX_Chain`
- Keep page and layer position memory
- Reattach a control when its physical knob moves beyond a small threshold
- Slew the effective value from the stored value to the new knob target over a
  short time instead of jumping immediately
- Update host-side tests to cover the new behavior
- Change `DAISY_QAE` soft-takeover wording from mandatory rule to recommendation
- Update firmware header comments and README

## Approved Approach

Use first-move reattach with timed slew.

Why:

- stored page values remain active immediately after switching pages
- the knob does not have to cross the old stored value
- the newly active control still avoids a hard jump
- behavior is simpler than full pickup/catch while remaining safe for audio

## Control Model

### Page and Layer Memory

- Every page keeps its last effective values
- Tube default and shift layers keep separate values
- LFO coarse and fine layers keep separate values
- When a page or layer becomes active again, its stored values remain in effect

### Reattach Rule

- On page or layer change, the active logical control is detached from the raw
  physical knob position
- The first update after a page/layer change only records the current raw knob
  position as a movement anchor
- When the raw knob moves beyond a small movement threshold, that logical
  control reattaches

### Slew Rule

- On reattach, the effective value does not jump to the raw target
- Instead, it slews toward the target at a fixed rate
- Approved recommendation: use a short live-performance-safe ramp rather than a
  full `1 s`
- Chosen design target: full-scale move reaches target in about `300 ms`

This is short enough to feel responsive on a pedal and long enough to avoid
obvious clicks or abrupt modulation changes.

## Helper State Model

Replace the soft-takeover `captured` flags with reattach and slew state per
logical knob.

Required per-knob state:

- current effective value
- current target value
- last raw anchor used for movement detection
- attached/detached state
- anchor-initialized flag

This keeps the helper Daisy-independent and testable.

## Runtime Integration

- Keep analog control reads in the audio callback
- Pass raw knob values and per-block elapsed time into the helper
- Use the helper's effective values when mapping Tube, Delay, Reverb, and LFO
  parameters
- Keep all existing page selection, encoder push, bypass, and LED semantics

## DAISY_QAE Update

The existing Seed/Field soft-takeover text should no longer be mandatory.

Replace it with a promemoria/recommendation:

- when reusing a physical control across multiple banks/pages/layers, avoid
  abrupt parameter jumps
- suggested patterns include:
  - pickup/catch (soft takeover)
  - first-move reattach with slew
  - equivalent smoothing strategy

## Verification Strategy

- Update the host-side test first so it fails on:
  - detached state after page/layer change
  - first-move reattach
  - intermediate slew progress
  - eventual arrival at the target value
- Re-run host-side test after helper and firmware updates
- Run Daisy QAE lint
- Run clean firmware rebuild
