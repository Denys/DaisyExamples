# Evidence

This directory stores raw and derived evidence without promoting a test simply because a file exists.

Recommended P0 structure:

```text
evidence/p0/<run-id>/
├── fixture.md
├── build.log
├── firmware.sha256
├── cold_boot_serial.log
├── controls.csv
├── cv_inputs.csv
├── cv_outputs.csv
├── gate_io.csv
├── midi.log
├── audio/
│   ├── method.md
│   └── raw-captures...
└── timing/
    ├── method.md
    └── raw-captures...
```

Each run should state one of `VERIFIED`, `DERIVED`, `PROPOSED`, `ASSUMED`, `HOLD`, or `NOT_RUN` for every claimed result.

Do not commit commercial manuals or copyrighted factory-pattern books merely to make a provenance folder look busy.
