# ExtControls — Daisy Pod + OLED-Rotary-Encoder Module

Demonstrates integration of an external SSD1306 OLED + EC11 rotary encoder module with a Daisy Pod (old DIP-socket revision). Includes a hierarchical menu UI, coexistence with Pod onboard controls, and an audio passthrough skeleton.

---

## Hardware Wiring

### OLED-Rotary-Encoder Module → Daisy Pod (old DIP-socket revision)

| Module Pin | Wire Color | Seed Pin | Daisy ID      | Function                          |
|------------|------------|----------|---------------|-----------------------------------|
| VCC        | Red        | P38      | `3V3_D`       | 3.3 V digital supply              |
| GND        | Black      | P40      | `DGND`        | Ground                            |
| SDA        | Yellow     | P13      | `seed::D12`   | I²C1 SDA (I2C1_SDA)              |
| SCL        | Orange     | P12      | `seed::D11`   | I²C1 SCL (I2C1_SCL)              |
| TRA        | White      | P8       | `seed::D7`    | EC11 encoder channel A            |
| TRB        | Gray       | P9       | `seed::D8`    | EC11 encoder channel B            |
| PSH        | Brown      | P10      | `seed::D9`    | Encoder push-button (active-LOW)  |
| CON        | Green      | P11      | `seed::D10`   | CONFIRM button / KEY1 (active-LOW)|
| BAK        | Cherry     | P24      | `seed::D17`   | BACK button / KEYO (active-LOW)   |

> **BAK alternative**: If you observe a conflict with Pod LED2_R on the **new** Pod revision, add `#define BAK_USE_D16` to `ExtEncoder.h` (or to the Makefile via `-DBAK_USE_D16`) to move BAK to `seed::D16` (P23/A1). The old DIP-socket Pod does **not** route LED2 through the board, so D17 should be safe.

### I²C Notes

- OLED I²C address: `0x3C` (default). Change to `0x3D` in `OledUI.h` if the ADDR pad is bridged high.
- The module has 4.7 kΩ pull-ups onboard — no external pull-ups required.
- Only one device on I2C_1 bus; no address conflicts expected.

### SPI Conflict Warning

Pins D7–D10 (TRA/TRB/PSH/CON) overlap with SPI1. Do **not** call `SpiHandle::Init()` or any SPI peripheral that claims these pins.

---

## Pod Onboard Controls

| Control       | Daisy ID    | API                              |
|---------------|-------------|----------------------------------|
| Pod encoder   | D26/D25/D13 | `pod.encoder`                    |
| SW1           | D27         | `pod.button1`                    |
| SW2           | D28         | `pod.button2`                    |
| POT1          | D21 / A6    | `pod.GetKnobValue(DaisyPod::KNOB_1)` |
| POT2          | D15 / A0    | `pod.GetKnobValue(DaisyPod::KNOB_2)` |
| LED1 RGB      | D20/D19/D18 | `pod.led1.Set(r, g, b)`         |
| LED2 RGB      | D17/D24/D23 | `pod.led2.Set(r, g, b)`         |

Pod encoder scrolls and confirms the menu (dual control with the external encoder).

---

## Menu Structure

```
MAIN MENU
├── Patch Select     → Basic Synth / Delay Echo / Reverb Pad / Filter Sweep
├── Params           → Bar-graph display of 4 parameters (set by pots)
├── MIDI             → Channel (1–16) / Thru (ON/OFF)
└── System           → Invert Enc (ON/OFF) / Disp Timeout (Off/5s/10s/30s)
```

### Navigation Controls

| Action                        | Control                        |
|-------------------------------|--------------------------------|
| Scroll items / adjust value   | External encoder rotate        |
| Enter item / confirm value    | PSH (encoder push) OR CON      |
| Go back / cancel edit         | BAK button                     |
| Jump to root menu             | PSH held > 500 ms              |
| Dual scroll (fallback)        | Pod encoder rotate             |
| Dual confirm (fallback)       | Pod encoder push               |

---

## Build Instructions

**Prerequisites**

- `arm-none-eabi-g++` toolchain (≥ 10.x)
- libDaisy and DaisySP checked out as siblings of `MyProjects/`:
  ```
  DaisyExamples/
  ├── libDaisy/
  ├── DaisySP/
  └── MyProjects/_projects/Pod_OledEncoder_test/   ← this project
  ```
- ST-Link V2/V3 programmer connected (for `make program`)

**Build and flash**

```bash
cd DaisyExamples/MyProjects/_projects/Pod_OledEncoder_test
make clean && make
make program          # ST-Link (default, recommended)
make program-dfu      # DFU bootloader (hold BOOT, tap RESET, release BOOT)
```

---

## Known Issues and Notes

### Encoder direction reversed

If the encoder counts backwards (CW decrements instead of increments), either:

1. Toggle **System → Invert Enc** in the menu (runtime, no re-wiring).
2. Or call `ext.SetFlipped(true)` at init in `ExtControls.cpp`.

### SSD1306 vs SH1106

The driver in `OledUI.h` is `SSD130xI2c128x64Driver` which targets SSD1306 128×64. Some similar blue modules use an SH1106 controller, which requires a different driver (`SH110xI2c128x64Driver` in libDaisy). If the display shows garbled output or a column offset, swap the driver type alias in `OledUI.h`:

```cpp
// SSD1306 (default)
using MyDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

// SH1106 (alternative)
// using MyDisplay = daisy::OledDisplay<daisy::SH110xI2c128x64Driver>;
```

### libDaisy Encoder::Init update_rate parameter

If the build fails with a "too many arguments" error on `enc_.Init(...)`, the libDaisy version you have deprecated the `update_rate` parameter. Remove it:

```cpp
// If build fails, change in ExtEncoder.h::Init():
enc_.Init(TRA_PIN, TRB_PIN, PSH_PIN);   // no update_rate
```

### BAK pin conflict on new Pod revision

On the **new** Pod revision, `seed::D17` is routed to LED2_R. Add `-DBAK_USE_D16` to the Makefile's `C_DEFS` to use `seed::D16` (P23) instead:

```makefile
C_DEFS += -DBAK_USE_D16
```

---

## Firmware Architecture

### Why the display is NOT updated in the audio callback

The SSD1306 communicates via I²C. `display_.Update()` is a **blocking** function that transfers 1 KB of framebuffer data over the bus (~2 ms at 400 kHz). Calling any I²C function inside `AudioCallback` would:

1. Block the DMA audio interrupt for multiple milliseconds, causing audible glitches and dropouts.
2. Violate real-time guarantees required by the audio subsystem.

All display and GPIO reads therefore happen exclusively in the **main loop**, rate-limited to ≤ 30 fps. The audio callback reads only pre-latched `param[]` values written by the main loop.

### Threading model (single-core, no RTOS)

```
┌─────────────────────────────────────────────────────┐
│ Main Loop (background, no priority)                  │
│  • Debounce controls @ 1 kHz (timer-checked)        │
│  • Process menu input                               │
│  • Read pots → update param[]                       │
│  • Draw OLED @ 30 fps                               │
│  • Update LEDs                                      │
├─────────────────────────────────────────────────────┤
│ DMA Audio Interrupt (highest priority)               │
│  • AudioCallback: read param[], produce audio        │
│  • NO I²C, NO GPIO reads, NO malloc, NO printf      │
└─────────────────────────────────────────────────────┘
```

`param[]` is declared `volatile float` to prevent the compiler from caching stale values across the interrupt boundary.
