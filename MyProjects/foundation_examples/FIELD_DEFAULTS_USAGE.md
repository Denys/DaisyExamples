# field_defaults.h Usage Guide

## Overview

`field_defaults.h` provides standard constants and helpers for Daisy Field projects to eliminate repetitive code.

For banked parameters and pickup/catch control, also include `field_parameter_banks.h`.
For 3-state key LED handling, include `field_instrument_ui.h` and use `KeyLedState`
with `FieldTriStateKeyLEDs`.

For any Field project with an OLED or other display, start from
`FIELD_DISPLAY_PROJECT_README_TEMPLATE.md` and keep the project README aligned
with the actual controls, LED states, OLED pages, hidden banks, startup/default
values, and panic/reset behavior.

## Documentation Standard

Display-based Field projects should document the following in their README:

- All live controls and their exact roles
- LED states for knobs, keys, switches, and mode indicators
- OLED pages, including hidden or alternate pages
- Hidden banks, shift layers, or modifier states
- Startup/default values for all important parameters, modes, and states
- Panic/reset behavior and recovery from stuck notes or bad states
- Whether keybed controls use `Off`, `Blink`, and `On` as a 3-state vocabulary

When a keybed is repurposed for controls instead of notes, prefer a tri-state
LED helper so the control state is readable at a glance. Use the LED states
that best match the control semantics, but document the mapping explicitly.

## Quick Start

```cpp
#include "daisy_field.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_parameter_banks.h"
#include "../../foundation_examples/field_instrument_ui.h"

using namespace daisy;
using namespace FieldDefaults;
using namespace FieldParameterBanks;
using namespace FieldInstrumentUI;

DaisyField hw;
FieldKeyboardLEDs keyLeds;  // Toggle LED helper
ParamBankSet paramBanks;    // Main/alt parameter storage with pickup/catch
FieldTriStateKeyLEDs triKeyLeds; // Off / Blink / On state LEDs

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    hw.ProcessAllControls();
    
    // Your audio processing here...
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    keyLeds.Init(&hw);
    triKeyLeds.Init(&hw);
    paramBanks.Init(0.5f);
    paramBanks.SetActiveBank(ParamBank::Main);
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    while(1)
    {
        // Toggle keyboard LEDs on key press
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
                keyLeds.ToggleA(i);  // Toggle A row LED
                
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
                keyLeds.ToggleB(i);  // Toggle B row LED
        }

        // Update all keyboard LEDs
        keyLeds.Update();
        
        System::Delay(1);
    }
}
```

**Important:** `FieldKeyboardLEDs` and `FieldTriStateKeyLEDs` are alternative
helpers for the same keyboard LED matrix. Use one or the other in a given loop,
not both at once, unless you explicitly split their responsibilities.

## What's Included

### 1. Hardware Constants
```cpp
kNumKnobs = 8
kNumKeys = 16
kNumKeysPerRow = 8
kNumSwitches = 2
kNumCVInputs = 4
kRecommendedBlockSize = 48
```

### 2. LED Arrays
```cpp
kLedKnobs[8]         // LED_KNOB_1 through LED_KNOB_8
kLedKeysA[8]         // LED_KEY_A1 through LED_KEY_A8 (top row)
kLedKeysB[8]         // LED_KEY_B1 through LED_KEY_B8 (bottom row)
kLedSwitches[2]      // LED_SW_1, LED_SW_2
kLedKeysPlayable[13] // 13 playable keys (excludes black key gaps)
```

**Example - Set knob LEDs to match knob values:**
```cpp
for(int i = 0; i < 8; i++)
{
    float value = hw.knob[i].Process();
    hw.led_driver.SetLed(kLedKnobs[i], value);
}
hw.led_driver.SwapBuffersAndTransmit();
```

### 3. Keyboard Index Mappings

**Important:** The Field defaults map rows in natural order:

```
Physical Layout:
  A1  A2  A3  A4  A5  A6  A7  A8   (Top Row)
  B1  B2  B3  B4  B5  B6  B7  B8   (Bottom Row)

Array Indices:
   0   1   2   3   4   5   6   7   (kKeyAIndices)
   8   9  10  11  12  13  14  15   (kKeyBIndices)
```

**Example - Check if key A4 was pressed:**
```cpp
if(hw.KeyboardRisingEdge(kKeyAIndices[3]))  // A4 = index 3
{
    // Key A4 was pressed
}
```

### 4. Toggle LED Helper (FieldKeyboardLEDs)

Provides automatic toggle behavior for keyboard LEDs:
- **First press** = LED turns ON
- **Second press** = LED turns OFF

**Methods:**
```cpp
keyLeds.Init(&hw);           // Initialize with hardware
keyLeds.ToggleA(0-7);        // Toggle LED for keys A1-A8
keyLeds.ToggleB(0-7);        // Toggle LED for keys B1-B8
keyLeds.SetA(idx, true/false); // Set LED state directly
keyLeds.SetB(idx, true/false); // Set LED state directly
keyLeds.GetA(idx);           // Get current LED state
keyLeds.GetB(idx);           // Get current LED state
keyLeds.Clear();             // Turn off all LEDs
keyLeds.Update(brightness);  // Update hardware (call every loop)
```

**Example - Toggle with custom brightness:**
```cpp
FieldKeyboardLEDs leds;
leds.Init(&hw);

// In main loop:
for(int i = 0; i < 8; i++)
{
    if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
        leds.ToggleA(i);
}

leds.Update(0.7f);  // 70% brightness
```

### 5. Musical Scales (Optional)

```cpp
kScaleMajor[16]      // C major scale mapped to keyboard
kScaleChromatic[12]  // Chromatic scale
```

**Example - Use major scale for synth:**
```cpp
int octave = 2;
for(int i = 0; i < 16; i++)
{
    if(hw.KeyboardRisingEdge(i) && i != 8 && i != 11 && i != 15)
    {
        float midi_note = (12.0f * octave) + 24.0f + kScaleMajor[i];
        // Trigger note...
    }
}
```

## Common Patterns

### Pattern 1: Keyboard LED Feedback
```cpp
FieldKeyboardLEDs leds;
bool key_states[16] = {false};

void UpdateKeyboardLEDs()
{
    for(int i = 0; i < 8; i++)
    {
        // A row
        if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            leds.ToggleA(i);
            
        // B row
        if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            leds.ToggleB(i);
    }
    leds.Update();
}
```

### Pattern 2: Knob LED Mirrors Knob Value
```cpp
void UpdateKnobLEDs()
{
    for(int i = 0; i < kNumKnobs; i++)
    {
        float value = hw.knob[i].Process();
        hw.led_driver.SetLed(kLedKnobs[i], value);
    }
    hw.led_driver.SwapBuffersAndTransmit();
}
```

### Pattern 3: Switch LED Status Indicators
```cpp
bool playing = false;
bool muted = false;

void UpdateSwitchLEDs()
{
    hw.led_driver.SetLed(kLedSwitches[0], playing ? 1.0f : 0.0f);
    hw.led_driver.SetLed(kLedSwitches[1], muted ? 1.0f : 0.0f);
    hw.led_driver.SwapBuffersAndTransmit();
}
```

### Pattern 4: Banked Parameters With Pickup / Catch
```cpp
ParamBankSet banks;
banks.Init(0.5f);
banks.SetActiveBank(ParamBank::Main);

// Store independent values for Main and Alt banks.
banks.Write(ParamBank::Main, 0, 0.50f);
banks.Write(ParamBank::Alt, 0, 0.35f);

// Read the active bank without mutating anything.
float active_value = banks.ReadActive(0);

// Catch the physical control before writing after a bank change.
if(banks.CatchIfCloseActive(0, physical_knob_value))
{
    banks.WriteActive(0, physical_knob_value);
}
```

### Pattern 5: Three-State Key LEDs
```cpp
FieldTriStateKeyLEDs triLeds;
triLeds.Init(&hw);

triLeds.SetA(0, KeyLedState::Off);
triLeds.SetA(1, KeyLedState::Blink);
triLeds.SetA(2, KeyLedState::On);
triLeds.Update(System::GetNow());
```

## Migration Guide

### Before (without field_defaults.h):
```cpp
const size_t knob_leds[] = {
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    // ... repeat for all 8 knobs
};

const int key_a_indices[8] = {15, 14, 13, 12, 11, 10, 9, 8};
const int key_b_indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
```

### After (with field_defaults.h):
```cpp
#include "../../foundation_examples/field_defaults.h"
using namespace FieldDefaults;

// Just use kLedKnobs, kKeyAIndices, kKeyBIndices directly!
```

### 6. OLED Display Helper (FieldOLEDDisplay)

Displays all settings with automatic highlighting of the most recently modified parameter.

**Features:**
- Tracks which parameter was last changed
- Two display layouts: Standard (highlighted active) and Compact (all 8 visible)
- Automatic change detection
- Customizable labels and title

**Methods:**
```cpp
display.Init(&hw);                    // Initialize
display.SetTitle("MyProject");        // Set top title
display.SetLabel(0, "Cutoff");        // Set parameter labels
display.SetValue(0, value);           // Update value (auto-detects changes)
display.SetActiveParam(2);            // Manually set active param
display.Update();                     // Standard layout
display.UpdateCompact();              // Compact layout (all 8 params)
```

**Example - Standard Layout (Shows Active Parameter Large):**
```cpp
FieldOLEDDisplay display;
display.Init(&hw);
display.SetTitle("Filter");

// Set labels for all 8 knobs
display.SetLabel(0, "Cutoff");
display.SetLabel(1, "Reso");
display.SetLabel(2, "Drive");
display.SetLabel(3, "Mix");
display.SetLabel(4, "Attack");
display.SetLabel(5, "Decay");
display.SetLabel(6, "Sustain");
display.SetLabel(7, "Release");

// In main loop:
for(int i = 0; i < 8; i++)
{
    float val = hw.knob[i].Process();
    display.SetValue(i, val);  // Automatically highlights when changed
}

display.Update();  // Shows active param large + first 2 params small
```

**Display Output (Standard):**
```
┌──────────────────┐
│ Filter           │  <- Title
│ > Cutoff         │  <- Active parameter name
│ 0.75             │  <- Active value (large)
│                  │
│ ---              │
│ Cutof:0.75       │  <- Param 0 (highlighted if active)
│ Reso:0.32        │  <- Param 1
└──────────────────┘
```

**Example - Compact Layout (Shows All 8 Parameters):**
```cpp
display.UpdateCompact();  // 2-column layout
```

**Display Output (Compact):**
```
┌──────────────────┐
│ Filter           │  <- Title
│ Cut:0.75  Att:0.5│  <- Params 0 & 4
│ Res:0.32  Dec:0.3│  <- Params 1 & 5
│ Drv:0.10  Sus:0.8│  <- Params 2 & 6
│ Mix:1.00  Rel:0.4│  <- Params 3 & 7
└──────────────────┘
```
The active parameter is inverted (white background).

## Complete Example - All Features Together

```cpp
#include "daisy_field.h"
#include "../../foundation_examples/field_defaults.h"

using namespace daisy;
using namespace FieldDefaults;

DaisyField hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay display;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    hw.ProcessAllControls();
    // Audio processing...
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("My Synth");
    
    // Label all knobs
    display.SetLabel(0, "Freq");
    display.SetLabel(1, "Cutoff");
    display.SetLabel(2, "Reso");
    display.SetLabel(3, "Drive");
    display.SetLabel(4, "Attack");
    display.SetLabel(5, "Decay");
    display.SetLabel(6, "Sustain");
    display.SetLabel(7, "Release");
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    bool compact_view = false;
    
    while(1)
    {
        // Toggle keyboard LEDs
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
                keyLeds.ToggleA(i);
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
                keyLeds.ToggleB(i);
        }
        keyLeds.Update();
        
        // Update knob values (auto-highlights changed param)
        for(int i = 0; i < 8; i++)
        {
            float val = hw.knob[i].Process();
            display.SetValue(i, val);
        }
        
        // Toggle view mode with Switch 1
        if(hw.sw[0].RisingEdge())
            compact_view = !compact_view;
        
        // Update display
        if(compact_view)
            display.UpdateCompact();
        else
            display.Update();
        
        System::Delay(16);
    }
}
```

## Notes

- The `FieldKeyboardLEDs` class manages toggle state internally - you don't need to track it yourself
- Remember to call `Update()` in every main loop iteration to push LED state to hardware
- All arrays use 0-based indexing (0-7 for each row of 8 keys/knobs)
- Keyboard physical layout is A1-A8 (top), B1-B8 (bottom)

