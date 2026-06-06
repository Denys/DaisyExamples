# Field_delay_daisy-sdram-delaylines

Daisy Field adaptation of `Farmer2K5/daisy-sdram-delaylines`, focused on the
source project's reusable external-buffer delay-line primitive: caller-owned
SDRAM buffers, fractional reads, long delay time, and stereo/ping-pong feedback.

Source cross-check:

- Current shallow source reviewed in `%TEMP%\daisy-delay-source-review\daisy-sdram-delaylines`
- Commit reviewed: `f3f7b2a`
- Evidence files: `include/DaisyFarm/delayline_ext.h`,
  `include/DaisyFarm/delayline_sdram.h`, `src/DaisyFarm/sdram_alloc.cpp`

Build:

```sh
cd MyProjects/_projects/Field_delay_daisy-sdram-delaylines
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
