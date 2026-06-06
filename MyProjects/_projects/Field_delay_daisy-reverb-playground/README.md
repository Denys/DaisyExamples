# Field_delay_daisy-reverb-playground

Daisy Field adaptation of `Farmer2K5/daisy-reverb-playground`, focused on the
source project's advanced delay-network ideas: early reflection, diffusion, FDN
tank behavior, damped feedback, and CPU-aware delay-cloud thinking.

Source cross-check:

- Current shallow source reviewed in `%TEMP%\daisy-delay-source-review\daisy-reverb-playground`
- Commit reviewed: `90c8497`
- Evidence files: `example_fdn_reverb.cpp`, `example_fdn_tank.cpp`,
  `include/DaisyFarm/firefly_delay_lite.h`,
  `include/DaisyFarm/firefly_delay_perf.h`

Build:

```sh
cd MyProjects/_projects/Field_delay_daisy-reverb-playground
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
