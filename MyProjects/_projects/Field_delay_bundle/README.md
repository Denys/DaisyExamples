# Field_delay_bundle

Daisy Field extended delay bundle that combines six source-verified delay/Fx
behavior families into one selectable project.

Algorithms:

- Tape [multifx] - adapted from `balazsbencs/daisy-multifx-pedal`
- Tank [reverb] - adapted from `Farmer2K5/daisy-reverb-playground`
- Texture [FunBox] - adapted from `GuitarML/FunBox`
- Long [sdram] - adapted from `Farmer2K5/daisy-sdram-delaylines`
- Spectral [Phantasmagoria] - clean-room behavior adaptation inspired by
  `FuzzyLotus/Phantasmagoria`
- 8 Tap [TimeMachine] - clean-room behavior adaptation inspired by
  `oamodular/time-machine`

`Phantasmagoria` is GPL-3.0 and `oamodular/time-machine` is CC BY-NC-SA 4.0,
so this project does not vendor or copy their source. The two extended modes
use source-inspected behavior classes: reverse-grain reading, smear diffusion,
erosion, chamber taps, freeze memory, eight read-head tap distribution, blur,
skew, and feedback limiting.

The bundle prioritizes a compilable Field target and shared DaisyHost behavior.
Each algorithm keeps its own parameter snapshot, so changing algorithm does not
erase the previous algorithm's knob settings.

The bundle includes an internal 8-voice pluck/pad resonator for testing and
performance without an external source. B1-B8 trigger C4-C5, external MIDI
uses the same voice engine, A1-A6 select algorithms, and A7/A8 shift the
keyboard octave.

Source cross-check:

- Current source reviews started from
  `C:\Users\denko\Codex\_weekly\Embedded_DSP_GitHub_Digest\docs\reports\2026-06-03-daisy-delay-source-verified-research.md`
- The implemented DSP core lives in
  `DaisyHost/include/daisyhost/DaisyDelayFxCore.h` and
  `DaisyHost/src/DaisyDelayFxCore.cpp`
- The shared Field adapter lives in
  `MyProjects/_projects/Field_delay_shared/FieldDelayFieldApp.h`
- The project-local block diagram lives in
  `MyProjects/_projects/Field_delay_bundle/Field_delay_bundle.dvpe`

Build:

```sh
cd MyProjects/_projects/Field_delay_bundle
make
```

Windows PowerShell build:

```powershell
make clean
make
```

Windows PowerShell DFU flash:

```powershell
make program-dfu
```

On this workstation the PowerShell profile exposes DaisyToolchain `make.exe`
and Git `rm.exe`, so Daisy's `make clean` target works. If a shell profile is
not loaded, remove `build` with PowerShell or run the full DaisyToolchain
`make.exe` path.

Validation:

```powershell
$env:PATH = 'C:\Program Files\DaisyToolchain\bin;' + $env:PATH
& 'C:\Program Files\DaisyToolchain\bin\make.exe'
$env:PYTHONIOENCODING = 'utf-8'
py -3 ../../../DAISY_QAE/validate_daisy_code.py .
```

This project is build-only by default. Hardware flashing is intentionally not
part of the normal validation command.
