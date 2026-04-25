# Field MultiDelay

`field/MultiDelay` is the first Daisy Field firmware parity adapter for
DaisyHost. It runs the shared `daisyhost::apps::MultiDelayCore` on real
Daisy Field hardware so the same regression fixture used by DaisyHost has a
hardware-facing Field path.

The Daisy Field is expected to be connected to the development computer through
an ST-Link programmer/debugger for flashing and hardware validation.

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

## Validation Status

This project may be marked hardware-validated only after:

- `make` succeeds
- `make program` succeeds through ST-Link
- the manual checklist in `CONTROLS.md` is completed and dated

Until then, describe the result as build-verified or flash-verified only.

## Scope

This adapter proves one narrow Field hardware path for MultiDelay. It does not
claim DAW/VST3 validation, mixed-board racks, DaisyHost routing changes, or a
general Field firmware framework.
