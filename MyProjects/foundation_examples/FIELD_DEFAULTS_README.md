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
| A1-A8 (Top Row) | 0-7 |
| B1-B8 (Bottom Row) | 8-15 |

### LED Indices (`hw.led_driver.SetLed()`)
| Physical LED | Index | Formula |
|--------------|-------|---------|
| A1 | 15 | `15 - position` |
| A2 | 14 | |
| A3 | 13 | |
| A4 | 12 | |
| A5 | 11 | |
| A6 | 10 | |
| A7 | 9 | |
| A8 | 8 | |
| B1 | 0 | `position` |
| B2 | 1 | |
| B3 | 2 | |
| B4 | 3 | |
| B5 | 4 | |
| B6 | 5 | |
| B7 | 6 | |
| B8 | 7 | |
| Knob 1-8 | 16-23 | `16 + position` |
| SW1, SW2 | 24, 25 | |

### Quick Reference Arrays
```cpp
kKeyAIndices  = {0, 1, 2, 3, 4, 5, 6, 7};       // A1-A8 input
kKeyBIndices  = {8, 9, 10, 11, 12, 13, 14, 15}; // B1-B8 input
kLedKeysA     = {15, 14, 13, 12, 11, 10, 9, 8}; // A1-A8 LEDs
kLedKeysB     = {0, 1, 2, 3, 4, 5, 6, 7};       // B1-B8 LEDs
```
