# INSTANCE: DAISY-FIRMWARE [SECONDARY]
> **NOT the DVPE App instance.** For TypeScript app Noderr → `noderr/noderr/`

| Field           | Value                                                              |
|-----------------|---------------------------------------------------------------------|
| Instance ID     | DAISY-FIRMWARE-SECONDARY                                            |
| Project         | Daisy Embedded Audio Firmware — C++ via NLP + DVPE pipeline         |
| Language        | C / C++ (ARM Cortex-M7, libDaisy + DaisySP)                        |
| Build command   | `make && make program` (ST-Link)                                    |
| Specs namespace | `DSP_`, `FX_`, `LIBDASY_`, `DVPE_`                                 |
| NodeID count    | 24 (17% verified)                                                   |
| Noderr path     | `DaisyExamples/noderr/`                                             |
| Mode selector   | `.claude/MODE_SELECTOR.md` → **Mode: Firmware**                     |
| Session starter | `START_FIRMWARE_SESSION.md` (at project root)                       |

**Wrong-instance check:** If your task involves `npm`, `React`, `.tsx`, or `UI_`/`SVC_` NodeIDs
— stop. Navigate to `noderr/noderr/` and read its `INSTANCE_IDENTITY.md` instead.
