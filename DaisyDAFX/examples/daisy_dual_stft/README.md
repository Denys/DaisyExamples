# Daisy Dual STFT Firmware Example

Board: Daisy Seed by default; pass `STFT_BOARD_FIELD` for Daisy Field.

Default profile: mono `512/128/64`, 48 kHz, Fast-style complex identity path.
The defaults are also available through explicit macros:
`STFT_BACKEND_FAST` and `STFT_PROFILE_512`. The firmware rejects conflicting
backend or profile macro combinations at compile time.

Build commands:

```sh
make
make STFT_DEFS="-DSTFT_BACKEND_FAST -DSTFT_PROFILE_512"
make STFT_DEFS="-DSTFT_BACKEND_DAFX -DSTFT_PROFILE_512"
make STFT_DEFS="-DSTFT_BACKEND_FAST -DSTFT_PROFILE_1024"
make STFT_DEFS="-DSTFT_BACKEND_DAFX -DSTFT_PROFILE_1024"

# Daisy Field variants for ST-Link hardware validation.
make STFT_DEFS="-DSTFT_BOARD_FIELD"
make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_512"
make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_512"
make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_1024"
make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_1024"
```

The audio callback only performs block DSP and CPU load timing. Serial output is
limited to boot metadata and once-per-second CPU max/avg/min reporting from the
main loop.

No flash target is run by default. Use `make program` or `make program-dfu`
only when hardware flashing is explicitly intended.

Field measurement runner:

```powershell
# Build the required Field 512 pair without flashing.
powershell -ExecutionPolicy Bypass -File ..\..\tools\measure_dual_stft_field.ps1

# Explicitly flash and capture the required Field 512 pair on COM3.
powershell -ExecutionPolicy Bypass -File ..\..\tools\measure_dual_stft_field.ps1 -Port COM3 -Program

# Flash and capture all four benchmark variants.
powershell -ExecutionPolicy Bypass -File ..\..\tools\measure_dual_stft_field.ps1 -Port COM3 -Program -Include1024
```

On Windows, the runner auto-detects the DaisyToolchain OpenOCD script folder
and passes a Make-safe `OCD_DIR` value for ST-Link programming. If OpenOCD is
installed elsewhere, pass `-OpenOcdScriptsDir path\to\openocd\scripts`.

If `-Port` names the ST-Link virtual COM port, the runner resolves the capture
port to the Daisy USB serial logger device (`VID_0483&PID_5740`) after each
flash. On this machine, ST-Link VCP is `COM3` and the flashed Daisy logger
appears as `COM5`.

CPU capture after flashing a variant:

```powershell
# List visible ports.
[System.IO.Ports.SerialPort]::GetPortNames()

# Capture one minute of runtime CPU and update the matching benchmark entry.
powershell -ExecutionPolicy Bypass -File ..\..\tools\capture_dual_stft_cpu.ps1 -Port COMx -Seconds 65 -UpdateBenchmark

# Or parse a saved serial-monitor log with the same firmware lines.
powershell -ExecutionPolicy Bypass -File ..\..\tools\capture_dual_stft_cpu.ps1 -InputLog path\to\serial.log -UpdateBenchmark
```

Run the capture once per flashed backend/profile variant. The script parses the
firmware boot metadata and `CPU max/avg/min %` lines from live serial or a saved
log, then updates
`docs/benchmarks/2026-06-05-dual-stft-results.json` only when real serial CPU
samples are present.
