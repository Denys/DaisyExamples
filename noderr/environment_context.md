# Environment Context

Profile Name: Windows Local — Daisy Firmware (DVPE Pipeline)
Environment ID: ENV-2026-03-02-windows-daisy-firmware
Last Updated: 2026-03-02T00:00:00Z
Validated By: AI Agent
Confidence Level: High

## Environment Metadata

environment:
  type: development
  provider: local
  lifecycle: persistent
  purpose: Daisy C++ firmware development via DVPE pipeline
  environment_focus: DEVELOPMENT local workspace only

## Development Versus Production

development_server:
  access_urls:
    local_dev_preview:
      url: N/A — embedded hardware development
      description: code is built locally then flashed to Daisy hardware via ST-Link
      how_to_access: cd into project directory, run make, then make program
    public_deployed_app:
      url: N/A — no web deployment
      description: production is a flashed binary on physical Daisy hardware
      warning: DO NOT flash unverified firmware to production hardware

## Verified Tooling

- git available
- make available (MINGW64 on Windows)
- arm-none-eabi-gcc available
- ST-Link / OpenOCD available (make program)
- workspace path: c:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples

## Verified Commands

```
git status
git log --oneline -3
make --version
arm-none-eabi-gcc --version
```

## Timestamp Command (Required for noderr_loop.md log entries)

```bash
date -u +"%Y-%m-%dT%H:%M:%SZ"
```

Example output: `2026-03-02T14:30:00Z`

Use this command to generate the `Timestamp` field in all `noderr_log.md` entries and spec file headers.

## Build and Flash Workflow

```bash
# 1. Navigate to the target project directory
cd DaisyExamples/MyProjects/_projects/<ProjectName>/

# 2. Clean and build
make clean && make

# 3. Flash via ST-Link (PRIMARY)
make program

# 4. Flash via DFU (FALLBACK — only if ST-Link unavailable)
make program-dfu

# 5. Verify behavior on development hardware
```

## Primary vs Fallback Programming Methods

| Method | Command | When to Use |
|--------|---------|-------------|
| ST-Link **(PRIMARY)** | `make program` | Default — faster, no mode switching required |
| DFU (FALLBACK) | `make program-dfu` | Only if ST-Link is unavailable |

## DVPE Pipeline Integration

Generated C++ firmware from the DVPE app is output as a ZIP, then built here:

1. DVPE app (`dvpe_CLD/`) exports ZIP: `main.cpp` + `Makefile`
2. Extract project into `DaisyExamples/MyProjects/_projects/<Name>/`
3. Build: `make clean && make`
4. Flash: `make program`

## Notes

- No browser-based preview URL — testing is build + hardware flash cycle
- Production = firmware currently flashed to physical hardware
- Audio callback must complete in < 20.83µs at 48kHz — no blocking operations
- No `malloc`, `new`, or `printf` inside `AudioCallback` — real-time constraint
- Always call `hw.StartAdc()` before `hw.StartAudio()` on Field/Pod platforms
