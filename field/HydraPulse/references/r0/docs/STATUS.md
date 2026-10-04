# Status

## Implemented in this bootstrap

Evidence class: **VERIFIED locally** only after the accompanying validation commands are run in the generated package.

- R0 pure C++ core files exist.
- Native test sources exist.
- CMake/CTest harness exists.
- Repository/provenance validators exist.
- GitHub CI definition exists.
- Upstream references are pinned to full inspected commit SHAs.
- P0 evidence structure and implementation contract exist.

## Future work / not claimed

- **NOT_RUN:** GitHub CI for this bootstrap until it is actually pushed.
- **NOT_RUN:** ARM/libDaisy build.
- **NOT_RUN:** Field firmware execution.
- **NOT_RUN:** all physical P0 measurements.
- **PROPOSED:** P1 four-voice synthesis and sonic regression.
- **PROPOSED:** A/B/Fill.
- **DEFERRED:** advanced sequencing.
- **DEFERRED:** Hydrasynth MIDI/CV/audio integration.

## Evidence vocabulary

Use only:

- `VERIFIED` - directly observed/measured in the stated fixture or executed test;
- `DERIVED` - computed from verified evidence with method shown;
- `PROPOSED` - design or intended implementation;
- `ASSUMED` - assumption needed to proceed and not yet verified;
- `HOLD` - blocked or evidence too weak;
- `NOT_RUN` - test or action has not been executed.
