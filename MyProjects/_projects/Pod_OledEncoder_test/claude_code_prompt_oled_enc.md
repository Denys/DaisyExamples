# Claude Code Prompt — Daisy Pod + OLED-Rotary-Encoder Module Integration

---

## OBJECTIVE

Implement a complete libDaisy C++ integration layer for the **OLED-Rotary-Encoder module** (EC11 + SSD1306) wired to a Daisy Pod (old DIP-socket revision). Produce a working, buildable project demonstrating:

1. `OledDisplay` I²C init and full rendering helper class
2. `Encoder` + two `GPIO` buttons with debounce
3. A hierarchical menu UI rendered on the OLED, navigated via the ext encoder and buttons
4. Co-existence with Daisy Pod onboard controls (Pod encoder, SW1/SW2, POT1/POT2, LEDs)
5. Audio passthrough skeleton that does not block UI

---

## HARDWARE CONTEXT

### Platform
- **Daisy Pod** (old revision with DIP socket header — no J1 breakout)
- Daisy Seed **Rev7** (STM32H750VBT6, PCM3060 codec, libDaisy)
- libDaisy / DaisySP — latest master

### OLED-Rotary-Encoder Module (the blue mini board)
| Module Pin | Wire Color | Seed Pin | Daisy ID     | Function            |
|-----------|-----------|----------|--------------|---------------------|
| CON       | Green     | P11      | `seed::D10`  | CONFIRM button (KEY1), active-LOW |
| SDA       | Yellow    | P13      | `seed::D12`  | I²C1 SDA (I2C1_SDA) |
| SCL       | Orange    | P12      | `seed::D11`  | I²C1 SCL (I2C1_SCL) |
| PSH       | Brown     | P10      | `seed::D9`   | Encoder push-button, active-LOW |
| TRA       | White     | P8       | `seed::D7`   | Encoder channel A (EC11 A-phase) |
| TRB       | Gray      | P9       | `seed::D8`   | Encoder channel B (EC11 B-phase) |
| BAK       | Cherry    | P24      | `seed::D17`  | BACK button (KEYO), active-LOW — **use P24/D17, NOT P7/D6** |
| GND       | Black     | P40      | `DGND`       | Ground |
| VCC       | Red       | P38      | `3V3_D`      | 3.3V digital supply ✅ |

**Critical notes:**
- BAK is on `seed::D17` (A2), NOT D6/SD_CLK — to preserve the SD card bus
- TRA/TRB: if encoder counts backwards, call `enc_ext.SetFlipped(true)` — no re-wiring
- I2C1 bus: `seed::D11`=SCL, `seed::D12`=SDA — native STM32 I2C1 peripheral
- OLED I²C address: `0x3C` (default; `0x3D` if ADDR pin high)
- The module's SSD1306 has 4.7 kΩ pull-ups onboard — no external pull-ups needed
- CON and BAK are labeled `CON(KEY1)` and `BAK(KEYO)` on the PCB silkscreen

### Pod Onboard Controls (already init'd by DaisyPod::Init())
| Control | Daisy ID | API |
|---------|----------|-----|
| Pod encoder A/B/SW | D26/D25/D13 | `pod.encoder` |
| SW1 | D27 | `pod.button1` |
| SW2 | D28 | `pod.button2` |
| POT1 | A6/D21 | `pod.GetKnobValue(0)` |
| POT2 | A0/D15 | `pod.GetKnobValue(1)` |
| LED1 RGB | D20/D19/D18 | `pod.led1` |
| LED2 RGB | D17/D24/D23 | `pod.led2` |

**Conflict check:** `seed::D17` is used for LED2_R in the new Pod (`pod.led2.SetRed()`). On the **old Pod (DIP socket revision)**, LED2 is not wired through the board — verify there is no conflict before using D17 for BAK. If conflict exists, use `seed::D16` (A1/P23) for BAK instead — adjust `BAK_PIN` define.

---

## DELIVERABLES

### File structure
```
ExtControls/
├── Makefile
├── ExtControls.cpp          # main application
├── ExtEncoder.h             # ext encoder + buttons wrapper
├── OledUI.h                 # OLED display + menu system
└── README.md
```

---

## IMPLEMENTATION SPECIFICATION

### 1. `ExtEncoder.h` — External Controls Wrapper

```cpp
// Wraps ext encoder (TRA/TRB/PSH) + CON + BAK buttons
// All pins are active-LOW with internal pull-ups

class ExtEncoder {
public:
    // Pin definitions — old Pod DIP socket
    static constexpr daisy::Pin TRA_PIN = daisy::seed::D7;   // P8
    static constexpr daisy::Pin TRB_PIN = daisy::seed::D8;   // P9
    static constexpr daisy::Pin PSH_PIN = daisy::seed::D9;   // P10
    static constexpr daisy::Pin CON_PIN = daisy::seed::D10;  // P11
    static constexpr daisy::Pin BAK_PIN = daisy::seed::D17;  // P24 (A2)

    void Init();
    void Debounce();  // call every ~1ms in main loop

    int8_t  Increment();        // +1 CW, -1 CCW, 0 unchanged
    bool    EncoderPressed();   // PSH just-pressed
    bool    EncoderHeld();      // PSH held > 500ms
    bool    ConfirmPressed();   // CON just-pressed
    bool    BackPressed();      // BAK just-pressed

private:
    daisy::Encoder enc_;   // handles TRA/TRB/PSH with built-in debounce
    daisy::Switch  con_;   // CON (CONFIRM) button
    daisy::Switch  bak_;   // BAK (BACK) button
};
```

**Implementation requirements:**
- `daisy::Encoder::Init(TRA_PIN, TRB_PIN, PSH_PIN, daisy::System::GetTickFreq())` — check if update_rate param is needed based on libDaisy version
- `daisy::Switch::Init(CON_PIN, 1000.0f, daisy::Switch::TYPE_MOMENTARY, daisy::Switch::POLARITY_INVERTED, daisy::Switch::PULL_UP)`
- Same for BAK
- `Debounce()` must call `enc_.Debounce()`, `con_.Debounce()`, `bak_.Debounce()` — all three, at 1kHz rate
- `Increment()` returns `enc_.Increment()`
- Button queries use `con_.RisingEdge()`, `bak_.RisingEdge()`, `enc_.RisingEdge()`, `enc_.Pressed()`

---

### 2. `OledUI.h` — OLED Display + Menu Engine

#### Display init
```cpp
// Type alias — I²C 128×64 SSD1306
using MyDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

// Init config:
MyDisplay::Config cfg;
cfg.driver_config.transport_config.i2c_config.periph   = daisy::I2CHandle::Config::Peripheral::I2C_1;
cfg.driver_config.transport_config.i2c_config.speed    = daisy::I2CHandle::Config::Speed::I2C_400KHZ;
cfg.driver_config.transport_config.i2c_config.pin_config.scl = daisy::seed::D11;
cfg.driver_config.transport_config.i2c_config.pin_config.sda = daisy::seed::D12;
cfg.driver_config.transport_config.i2c_address         = 0x3C;
display.Init(cfg);
```

#### Menu system design
Implement a lightweight **hierarchical menu** with the following structure:

```
[ROOT MENU]
├── Patch Select     → submenu: list of 4 patch names
├── Params           → submenu: 4 params (pot-overrideable)
├── MIDI             → submenu: Channel, Thru on/off
└── System           → submenu: Invert enc direction, Display timeout
```

Menu navigation:
| Action | Control |
|--------|---------|
| Scroll items | Ext encoder rotate |
| Enter item / confirm value | PSH (encoder push) OR CON button |
| Go back / cancel | BAK button |
| Adjust selected value | Ext encoder rotate (in edit mode) |
| Quick-exit to root | BAK held > 1s |

#### Display layout (128×64, monochrome)
```
┌────────────────────────────────┐  ← pixel (0,0) top-left
│ [TITLE BAR 8px tall]           │  ← page title, inverted (white bg)
│ > Item 1                       │  ← cursor indicator
│   Item 2                       │
│   Item 3                       │
│   Item 4                       │
│ [STATUS BAR 8px tall]          │  ← POT1 value | patch name
└────────────────────────────────┘
```

Rendering requirements:
- Font: `daisy::Font_6x8` (built-in libDaisy font)
- Cursor: `>` prefix on selected line; invert selected line (`display.DrawRect()` + white text)
- `display.Update()` called at ≤30 fps (every ~33ms) — NOT in audio callback
- Dirty flag: only redraw when state changes or every 500ms refresh

#### Parameter display (when in Params submenu):
```
┌────────────────────────────────┐
│ PARAMS                         │
│ Delay Time   [████████░░] 0.75 │
│ Feedback     [█████░░░░░] 0.50 │
│ Mix          [██░░░░░░░░] 0.20 │
│ Rate         [██████░░░░] 0.60 │
│ POT1: 0.75   POT2: 0.50        │
└────────────────────────────────┘
```
Bar width = floor(value × 20) pixels in a 20-char field. Draw with `DrawRect`.

---

### 3. `ExtControls.cpp` — Main Application

```cpp
// Skeleton structure
#include "daisy_pod.h"
#include "daisysp.h"
#include "ExtEncoder.h"
#include "OledUI.h"

using namespace daisy;
using namespace daisysp;

DaisyPod  pod;
ExtEncoder ext;   // external OLED module controls
OledUI    ui;     // display + menu

// Audio state
float param[4] = {0.5f, 0.5f, 0.5f, 0.5f};
int   patch_idx = 0;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t size) {
    // NEVER call display or GPIO reads here
    // Only read pre-latched values (param[])
    for(size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i];  // passthrough skeleton
        out[1][i] = in[1][i];
    }
}

int main() {
    pod.Init();
    pod.SetAudioBlockSize(4);
    pod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    ext.Init();
    ui.Init();   // inits display + menu structure

    pod.StartAudio(AudioCallback);
    pod.StartAdc();

    uint32_t last_debounce = 0;
    uint32_t last_draw     = 0;

    while(true) {
        uint32_t now = System::GetNow();

        // 1kHz debounce (1ms tick)
        if(now - last_debounce >= 1) {
            last_debounce = now;
            pod.ProcessAllControls();   // Pod onboard: encoder, buttons, knobs
            ext.Debounce();             // Ext module: encoder, CON, BAK
        }

        // Process ext inputs → drive menu
        int8_t inc = ext.Increment();
        if(inc != 0)          ui.Scroll(inc);
        if(ext.EncoderPressed() || ext.ConfirmPressed())  ui.Confirm();
        if(ext.BackPressed())  ui.Back();
        if(ext.EncoderHeld())  ui.BackToRoot();

        // Also drive menu from Pod encoder (optional dual control)
        int8_t pod_inc = pod.encoder.Increment();
        if(pod_inc != 0)  ui.Scroll(pod_inc);

        // Read pots → update params (and display)
        param[0] = pod.GetKnobValue(DaisyPod::KNOB_1);
        param[1] = pod.GetKnobValue(DaisyPod::KNOB_2);

        // Display update at 30fps
        if(now - last_draw >= 33) {
            last_draw = now;
            ui.Draw(param, patch_idx);
        }

        // LED feedback: pulse LED1 on enc press, LED2 color = patch
        if(ext.EncoderPressed()) {
            pod.led1.Set(0.f, 1.f, 0.f);
            pod.UpdateLeds();
        }
    }
}
```

---

### 4. `README.md`

Include:
- Hardware wiring table (exact pin table from above)
- Build instructions: `make` / `make program-dfu`
- Known issues: encoder direction invert compile flag, SSD1306 vs SH1106 detection
- Firmware architecture notes: why display is NOT updated in audio callback

---

## API REFERENCE (for Claude Code to use)

### Encoder Init (libDaisy current)
```cpp
// daisy::Encoder
void Init(Pin a, Pin b, Pin sw, float update_rate = 0.f);
int  Increment();      // +1/-1/0
bool Pressed();        // held
bool RisingEdge();     // just pressed
bool FallingEdge();    // just released
void Debounce();
```

### Switch Init (libDaisy current)
```cpp
// daisy::Switch
void Init(Pin p,
          float update_rate = 1000.f,
          Type  t           = TYPE_MOMENTARY,
          Polarity pol      = POLARITY_INVERTED,
          Pull pu           = PULL_UP);
bool RisingEdge();
bool FallingEdge();
bool Pressed();
void Debounce();
```

### OledDisplay Init pattern (I²C, libDaisy)
```cpp
using MyDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;
MyDisplay display;

// Config (exact field path — verified from oled_ssd130x.h):
MyDisplay::Config disp_cfg;
auto& i2c = disp_cfg.driver_config.transport_config.i2c_config;
i2c.periph                 = I2CHandle::Config::Peripheral::I2C_1;
i2c.speed                  = I2CHandle::Config::Speed::I2C_400KHZ;
i2c.pin_config.scl         = seed::D11;
i2c.pin_config.sda         = seed::D12;
disp_cfg.driver_config.transport_config.i2c_address = 0x3C;
display.Init(disp_cfg);
```

### Display drawing
```cpp
display.Fill(false);                          // clear (false = black)
display.SetCursor(x, y);
display.WriteString("text", Font_6x8, true); // true = white pixel
display.DrawRect(x, y, x2, y2, true, true);  // filled rect
display.DrawLine(x0, y0, x1, y1, true);
display.Update();                             // flush to device via I²C
```

### Makefile template
```makefile
TARGET = ExtControls
SOURCES = ExtControls.cpp
C_INCLUDES = -I$(LIBDAISY_DIR)/src/ -I$(LIBDAISY_DIR)/src/dev/
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR  = ../../DaisySP
include $(LIBDAISY_DIR)/core/Makefile
```

---

## CONSTRAINTS & GUARDRAILS

1. **No I²C calls inside AudioCallback** — display and I²C are blocking; all display ops in main loop only
2. **No dynamic allocation** — no `new`, no `std::vector`, no heap; all state in static/global arrays
3. **Menu max depth: 3 levels** — root → category → item edit
4. **Max menu items per page: 4** — fits 128×64 with 8px title + 4×12px rows + 8px status
5. **Encoder direction**: implement `bool flipped_` flag, toggled from System submenu, applied to `Increment()` return sign
6. **Display timeout**: if `display_timeout_ms > 0`, blank display after idle period (OLED longevity); wake on any input
7. **I²C bus collision check**: only one device on I2C_1 (the OLED) — no address conflict expected at 0x3C
8. **libDaisy version guard**: check if `Encoder::Init` takes `update_rate` param — if build fails, remove it (deprecated)
9. **Compile-time BAK pin select**: `#define BAK_USE_D17` to switch between `seed::D17` and `seed::D16` if old Pod LED2 conflict
10. **No SPI usage on D7–D10** — SPI1 must not be initialized; these pins repurposed as pure GPIO

---

## EXPECTED OUTPUT

Claude Code should produce:
1. `ExtControls.cpp` — compiles cleanly against libDaisy master + DaisySP
2. `ExtEncoder.h` — complete implementation, no stubs
3. `OledUI.h` — complete menu engine + display rendering, no stubs
4. `Makefile` — ready to `make` from `DaisyExamples/` sibling directory
5. `README.md` — wiring table + build steps

All files must compile with `arm-none-eabi-g++` at `-std=c++17` without warnings.
