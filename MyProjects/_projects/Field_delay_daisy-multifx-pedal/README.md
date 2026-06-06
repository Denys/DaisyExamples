# Field_delay_daisy-multifx-pedal

Daisy Field adaptation of `balazsbencs/daisy-multifx-pedal`, focused on the
source project's SDRAM tape-delay behavior: long delay memory, smooth delay-time
movement, feedback tone shaping, modulation, and grit.

Source cross-check:

- Current shallow source reviewed in `%TEMP%\daisy-delay-source-review\daisy-multifx-pedal`
- Commit reviewed: `294743e`
- Evidence files: `src/dsp/delay_line_sdram.cpp`,
  `src/modes/tape_delay.cpp`, `src/modes/nonlinear_reverb.cpp`

Build:

```sh
cd MyProjects/_projects/Field_delay_daisy-multifx-pedal
make
```

Validation:

```powershell
$env:PATH = 'C:\Program Files\DaisyToolchain\bin;' + $env:PATH
& 'C:\Program Files\DaisyToolchain\bin\make.exe'
$env:PYTHONIOENCODING = 'utf-8'
py -3 ../../../DAISY_QAE/validate_daisy_code.py .
```

This project is build-only by default. Hardware flashing is intentionally not
part of the normal validation command.
