# Field MultiDelay

`field/MultiDelay` is the first generated Daisy Field firmware adapter for
DaisyHost. It runs the shared `daisyhost::apps::MultiDelayCore`
on real Daisy Field hardware so the same app core used by DaisyHost has a
hardware-facing Field path.

The Daisy Field is expected to be connected to the development computer through
an ST-Link programmer/debugger for flashing and hardware validation.

## Build

```sh
make
```

## Flash

```sh
make program
```

## Validation Status

Generated firmware may be described as build-verified or flash-verified only
after the corresponding command passes. Full hardware validation still requires
the manual checklist in `CONTROLS.md`.

Until that checklist is complete, describe this generated adapter as
flash-verified only when `make program` has actually reported verification
success.
