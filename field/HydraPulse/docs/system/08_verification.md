# Verification

A PASS belongs to a specific executed test on a specific artifact. Host tests cover software
contracts, not ARM timing, codec gain, analog voltage or instrument ergonomics. The contract
stub compiles source against locally declared API shapes and must never be reported as
libDaisy or ARM validation. The target script refuses to substitute a native compiler.

Hardware records use VERIFIED, DERIVED, PROPOSED, ASSUMED, HOLD or NOT_RUN.
A VERIFIED hardware row requires fixture identity, firmware hash, raw artifact paths and
measurement method. Simulated waveforms cannot occupy measured-hardware fields.
Public CI never runs DFU/OpenOCD/programming commands. Missing toolchain or missing
physical hardware holds those gates without erasing completed source work.

```mermaid
flowchart TD
    Src["Candidate source + original presets"] --> Host["Native CMake / CTest"]
    Src --> Python["Schema / source / preservation validators"]
    Host --> San["ASan + UBSan / negative tests"]
    Host --> WAV["Deterministic host WAV scenarios"]
    Src --> Contract["Host adapter stub syntax check"]
    Contract -. "does not imply ARM success" .-> ARM["Real ARM + libDaisy link"]
    Pins["Exact dependency checkout and toolchain"] --> ARM
    ARM --> Binary["ELF + BIN + MAP + hashes + size gate"]
    Binary --> Flash["Explicit manual flash"]
    Flash --> P0["Measured Field Truth fixture"]
    P0 --> P1["Measured Beat and tutorial acceptance"]
    WAV --> Compare["Host / target numerical and audio comparison"]
    P1 --> Compare
    Compare --> Release["Candidate acceptance decision"]
    Remote["Authenticated target repo inspection"] --> Merge["Additive integration / no sibling changes"]
    Merge --> CI["CI on exact commit"]
    CI --> Release
```
