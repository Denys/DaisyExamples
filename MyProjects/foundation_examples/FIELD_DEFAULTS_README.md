# Field Defaults Summary

## What is this?

`field_defaults.h` is a comprehensive helper library for Daisy Field projects that provides:

1. **LED Constants** - Pre-defined arrays for all 21 LEDs (knobs, keyboard, switches)
2. **Keyboard Mappings** - Correct index mappings for the natural-order A/B row layout
3. **Toggle LED Helper** - `FieldKeyboardLEDs` class for on/off keyboard LED behavior
4. **OLED Display Helper** - `FieldOLEDDisplay` class showing all settings with active parameter highlighting
5. **Hardware Constants** - Standard values (8 knobs, 16 keys, block size 48, etc.)

## Files

- **field_defaults.h** - Main header file with all constants and classes
- **FIELD_DEFAULTS_USAGE.md** - Comprehensive usage guide with examples
- **FIELD_DISPLAY_PROJECT_README_TEMPLATE.md** - Shared README template for display-based Field projects

## Quick Example

```cpp
#include "../../foundation_examples/field_defaults.h"
using namespace FieldDefaults;

DaisyField hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay display;

int main(void)
{
    hw.Init();
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Synth");
    display.SetLabel(0, "Cutoff");
    
    while(1)
    {
        // Toggle LEDs
        for(int i = 0; i < 8; i++) {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
                keyLeds.ToggleA(i);
        }
        keyLeds.Update();
        
        // Display knobs (auto-highlights changed param)
        for(int i = 0; i < 8; i++)
            display.SetValue(i, hw.knob[i].Process());
        display.Update();
    }
}
```

## Features

### FieldKeyboardLEDs
- Toggle on first press, off on second press
- Separate tracking for A and B rows
- Simple `ToggleA(i)` / `ToggleB(i)` API
- Automatic LED driver updates

### FieldOLEDDisplay
- Two layouts: Standard (large active param) and Compact (all 8 visible)
- Automatic change detection and highlighting
- Customizable labels and title
- Shows recently modified parameter prominently
- No manual tracking needed

## Benefits

**Before:**
- Copy/paste LED arrays in every project (20+ lines)
- Manually track keyboard LED states
- Rewrite OLED display code each time
- Confusing keyboard index mappings

**After:**
- Include one header file
- Use 3 helper classes
- Focus on your project logic
- Well-documented constants

## See Also

- [FIELD_DEFAULTS_USAGE.md](FIELD_DEFAULTS_USAGE.md) - Full documentation
- [FIELD_DISPLAY_PROJECT_README_TEMPLATE.md](FIELD_DISPLAY_PROJECT_README_TEMPLATE.md) - README template for display-based projects
- Field example projects in `_projects/` folder

---

## Hardware Mapping Reference (CONFIRMED WORKING)

### Keyboard Input Indices (`hw.KeyboardRisingEdge()`)
| Physical Key | Index |
|--------------|-------|
| A1-A8 (Top Row) | 8-15 |
| B1-B8 (Bottom Row) | 0-7 |

### LED Indices (`hw.led_driver.SetLed()`)
| Physical LED | Index | Formula |
|--------------|-------|---------|
| A1 | 0 | native `LED_KEY_B1` |
| A2 | 1 | native `LED_KEY_B2` |
| A3 | 2 | native `LED_KEY_B3` |
| A4 | 3 | native `LED_KEY_B4` |
| A5 | 4 | native `LED_KEY_B5` |
| A6 | 5 | native `LED_KEY_B6` |
| A7 | 6 | native `LED_KEY_B7` |
| A8 | 7 | native `LED_KEY_B8` |
| B1 | 15 | native `LED_KEY_A1` |
| B2 | 14 | native `LED_KEY_A2` |
| B3 | 13 | native `LED_KEY_A3` |
| B4 | 12 | native `LED_KEY_A4` |
| B5 | 11 | native `LED_KEY_A5` |
| B6 | 10 | native `LED_KEY_A6` |
| B7 | 9 | native `LED_KEY_A7` |
| B8 | 8 | native `LED_KEY_A8` |
| Knob 1-8 | 16-23 | `16 + position` |
| SW1, SW2 | 24, 25 | |

### Quick Reference Arrays
```cpp
kKeyAIndices  = {8, 9, 10, 11, 12, 13, 14, 15}; // A1-A8 input
kKeyBIndices  = {0, 1, 2, 3, 4, 5, 6, 7};       // B1-B8 input
kLedKeysA     = {0, 1, 2, 3, 4, 5, 6, 7};       // A1-A8 LEDs via native LED_KEY_B*
kLedKeysB     = {15, 14, 13, 12, 11, 10, 9, 8}; // B1-B8 LEDs via native LED_KEY_A*
```

Native `DaisyField::LED_KEY_A/B` enum names are not the same layer as the
project-facing physical A/B row names. The original `field/KeyboardTest` and
`field/modalvoice` examples light the bottom playable row with native
`LED_KEY_A*` values, while public keyboard scan indices `0..7` are that
bottom row.
