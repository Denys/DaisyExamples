# Field_delay_FunBox

Daisy Field adaptation of `GuitarML/FunBox`, focused on the source family's
delay coverage: Mars two-tap delay, Saturn spectral-delay ideas, Uranus
granular-delay motion, and Pluto looper/freeze/reverse behavior.

Source cross-check:

- Current shallow source reviewed in `%TEMP%\daisy-delay-source-review\FunBox`
- Commit reviewed: `194d705`
- Evidence files: `software/Mars/mars.cpp`, `software/Saturn/saturn.cpp`,
  `software/Uranus/uranus.cpp`, `software/Pluto/pluto.cpp`

Build:

```sh
cd MyProjects/_projects/Field_delay_FunBox
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
