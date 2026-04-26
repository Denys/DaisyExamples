# DaisyHostController

`DaisyHostController` turns a Daisy Field into a USB MIDI controller for
DaisyHost. It is controller firmware only: it does not run an audio callback,
host a DaisyHost app core, or process audio.

The target sends standard MIDI CC and note messages so DaisyHost can use its
existing external MIDI input, MIDI tracker, and CC learn paths. No custom
DaisyHost protocol is required for this first pass.

## Build

From this directory:

```sh
make
```

## Flash

With the Daisy Field connected through ST-Link:

```sh
make program
```

## Mapping

See [CONTROLS.md](CONTROLS.md) for the MIDI map and manual validation
checklist.

## Validation Status

This project can be marked hardware-validated only after:

- `make` succeeds
- `make program` succeeds through ST-Link
- the manual checklist in `CONTROLS.md` is completed and dated

Latest flash evidence from 2026-04-26: `make program` passed through OpenOCD
with STLINK V3, target voltage `3.238171`, `** Verified OK **`, and target
reset.

Latest manual hardware note from 2026-04-26: knobs, buttons, and keys displayed
correctly on the Field screen. Field-to-computer USB MIDI was not checked.

Until the manual checklist is completed, describe the result as flash-verified
only.
