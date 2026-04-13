# Complete Plaits SD Card Data Tutorial

## Overview

This tutorial explains how to extract Plaits synthesis data from the source code and prepare it for loading from an SD card on Daisy Field, enabling additional synthesis engines.

**Current Status**: Field_MI_Plaits firmware has 9 engines working (99.99% flash)
**Goal**: Enable 16 engines by loading data from SD card

---

## Part 1: Understanding Plaits Data

### What Data is Needed

| Data | Size | Used By Engines |
|------|------|----------------|
| `wav_integrated_waves` | 49,920 bytes | Wavetable, Chord |
| `lut_fold` | 1,032 bytes | Waveshaping |
| `lut_fold_2` | 1,032 bytes | Waveshaping |

### Where Data Lives

The data is embedded in `PlaitsPatchInit/eurorack/plaits/resources.cc`:

```
Line 1273+: wav_integrated_waves[] = { -20784, -20822, ... }  (49,920 int16_t values)
Line ~65:   lut_fold[] = { ... }                          (516 float values)
Line ~73:   lut_fold_2[] = { ... }                    (516 float values)
```

---

## Part 2: Extract Data on Desktop

### Step 2.1: Create Extraction Script

Create a file called `extract_plaits_data.py` on your desktop computer:

```python
#!/usr/bin/env python3
"""
Extract Plaits data tables from resources.cc for SD card loading
Run: python extract_plaits_data.py
"""

import re
import struct
import sys

# Read the source file
with open('PlaitsPatchInit/eurorack/plaits/resources.cc', 'r') as f:
    content = f.read()

# Extract wav_integrated_waves (int16_t array)
print("Extracting wav_integrated_waves...")
match = re.search(
    r'const int16_t wav_integrated_waves\[\] = \{(.*?)\};',
    content, re.DOTALL
)
if match:
    # Parse the array values
    values = [int(x.strip()) for x in match.group(1).split(',') if x.strip()]
    # Convert to little-endian 16-bit binary
    data = struct.pack(f'<{len(values)}h', *values)
    
    # Verify size
    print(f"  Found {len(values)} values ({len(data)} bytes)")
    if len(data) != 49920:
        print(f"  WARNING: Expected 49920 bytes, got {len(data)}")
    
    with open('plaits/wavetable.bin', 'wb') as f:
        f.write(data)
    print(f"  Saved to wavetable.bin ({len(data)} bytes)")
else:
    print("  ERROR: wav_integrated_waves not found!")

# Extract lut_fold (float array)  
print("\nExtracting lut_fold...")
match = re.search(
    r'const float lut_fold\[\] = \{(.*?)\};',
    content, re.DOTALL
)
if match:
    values = [float(x.strip()) for x in match.group(1).split(',') if x.strip()]
    # Convert float to 16-bit fixed point (Q15 format)
    data_int16 = [int(x * 32767) for x in values]
    data = struct.pack(f'<{len(data_int16)}h', *data_int16)
    
    print(f"  Found {len(values)} values, saved as {len(data)} bytes")
    with open('plaits/table_fold.bin', 'wb') as f:
        f.write(data)
    print(f"  Saved to table_fold.bin")
else:
    print("  ERROR: lut_fold not found!")

# Extract lut_fold_2
print("\nExtracting lut_fold_2...")
match = re.search(
    r'const float lut_fold_2\[\] = \{(.*?)\};',
    content, re.DOTALL
)
if match:
    values = [float(x.strip()) for x in match.group(1).split(',') if x.strip()]
    data_int16 = [int(x * 32767) for x in values]
    data = struct.pack(f'<{len(data_int16)}h', *data_int16)
    
    with open('plaits/table_fold2.bin', 'wb') as f:
        f.write(data)
    print(f"  Saved to table_fold2.bin ({len(data)} bytes)")
else:
    print("  ERROR: lut_fold_2 not found!")

print("\n✓ Extraction complete!")
print("Files created:")
print("  - plaits/wavetable.bin")
print("  - plaits/table_fold.bin")  
print("  - plaits/table_fold2.bin")
```

### Step 2.2: Run Extraction

1. Copy `PlaitsPatchInit/eurorack/plaits/resources.cc` to your desktop
2. Save the script above as `extract_plaits_data.py`
3. Run:
```bash
python extract_plaits_data.py
```

Expected output:
```
Extracting wav_integrated_waves...
  Found 49920 values (49920 bytes)
  Saved to wavetable.bin (49920 bytes)

Extracting lut_fold...
  Found 516 values, saved as 1032 bytes
  Saved to table_fold.bin

Extracting lut_fold_2...
  Saved to table_fold2.bin (1032 bytes)

✓ Extraction complete!
```

---

## Part 3: Prepare SD Card

### Step 3.1: Format SD Card

1. Insert SD card into computer
2. Format as FAT32 (not exFAT)
   - Windows: Right-click → Format → FAT32
   - Mac/Linux: `mkfs.vfat /dev/sdX1`

### Step 3.2: Create Folder Structure

Create the following folder and files on SD card:

```
SD Card:/
└── plaits/
    ├── wavetable.bin    (49,920 bytes)
    ├── table_fold.bin    (1,032 bytes)
    └── table_fold2.bin  (1,032 bytes)
```

### Step 3.3: Verify Files

Check file sizes match:
```
wavetable.bin   = 49,920 bytes
table_fold.bin  = 1,032 bytes  
table_fold2.bin = 1,032 bytes
```

---

## Part 4: Daisy Field Integration

### Step 4.1: Add SD Card Loader Files

The files `sd_card_data.cpp` and `sd_card_data.h` have been created in the project folder.

### Step 4.2: Update Makefile

Add to `Field_MI_Plaits/Makefile`:

```makefile
# After existing CPP_SOURCES
CPP_SOURCES += sd_card_data.cpp

# In C_INCLUDES section, add FatFS path:
C_INCLUDES += -I$(LIBDAISY_DIR)/Middlewares/Third_Party/FatFs/src
```

### Step 4.3: Update Field_MI_Plaits.cpp

Add include and initialization call:

```cpp
// At top of Field_MI_Plaits.cpp, add:
#include "sd_card_data.h"

// In main(), AFTER hw.Init() but BEFORE voice.Init():
int main() {
    hw.Init();
    
    // Initialize SD card and load Plaits data
    bool sd_loaded = PlaitsLoadSdData();
    if (sd_loaded) {
        // SD data loaded - extra engines become available
    }
    
    voice.Init();
    // ... rest of initialization
}
```

### Step 4.4: Enable Extra Engine Sources

In `Makefile`, add the extra engine source files:

```makefile
# Add these new engine files
CC_SOURCES += \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/waveshaping_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/wavetable_engine.cc \
$(PLAITS_ROOT)/eurorack/plaits/dsp/engine/chord_engine.cc
```

---

## Part 5: Modify Voice.cc (Advanced)

### Step 5.1: Understand the Current Setup

In `plaits/dsp/voice.cc`, the engines currently use embedded data:

```cpp
// Current: uses resources.cc embedded data
engines_.RegisterInstance(&wavetable_engine_, false, 0.6f, 0.6f);
```

### Step 5.2: Add Conditional Loading

Modify `voice.cc` to use SD data if available:

```cpp
// At top of voice.cc, add external declaration
extern "C" {
    extern int16_t sd_wav_integrated_waves[];
    extern float sd_lut_fold[];
    extern float sd_lut_fold_2[];
}

// In Voice::Init(), add conditional:
void Voice::Init(BufferAllocator* allocator) {
    engines_.Init();
    
    // Check if SD data is available
    bool use_sd = (sd_wav_integrated_waves != NULL);
    
    // Register engines - use SD data if loaded
    if (use_sd) {
        // Override with SD data pointers
    }
    // ... rest of engine registration
}
```

This is advanced - you may need to create alternative data pointer declarations.

---

## Part 6: Testing

### Step 6.1: Build

```bash
cd DaisyExamples/MyProjects/_projects/Field_MI_Plaits
make clean
make
```

### Step 6.2: Flash to Daisy Field

```bash
# Using Daisy Bootloader
dfu-util -D build/Field_MI_Plaits.hex
```

### Step 6.3: Verify

1. Power on Daisy Field with SD card inserted
2. Check OLED display:
   - If SD loaded: Shows "SD: Full data" or similar
   - If no SD: Shows "SD: Not loaded" (9 engines still work)

---

## Complete File Summary

| File | Location | Purpose |
|------|----------|---------|
| `extract_plaits_data.py` | Desktop | Extracts binary data from source |
| `wavetable.bin` | SD Card /plaits/ | Wavetable data (49KB) |
| `table_fold.bin` | SD Card /plaits/ | Waveshaping LUT 1 (1KB) |
| `table_fold2.bin` | SD Card /plaits/ | Waveshaping LUT 2 (1KB) |
| `sd_card_data.cpp` | Project | SD loader implementation |
| `sd_card_data.h` | Project | SD loader header |

---

## Troubleshooting

### Problem: SD card not loading
- Check FAT32 format (not exFAT)
- Verify folder is `/plaits/` (lowercase)
- Check file sizes are exact

### Problem: Build fails
- Ensure FatFS headers are included
- Check Makefile paths

### Problem: Only 9 engines work
- SD data not loading - check serial debug
- Files may be wrong format

---

## Alternative: Using Python on Daisy (Not Recommended)

If you can't extract on desktop, you could theoretically write a Python script to run on a computer with the SD card slot, but the desktop extraction is much simpler.

**Recommended workflow:** Desktop extraction → SD card → Daisy Field

---

## Quick Reference Commands

```bash
# On desktop
python extract_plaits_data.py

# On Daisy Field
make -C DaisyExamples/MyProjects/_projects/Field_MI_Plaits

# Flash
dfu-util -D build/Field_MI_Plaits.hex
```

---

End of tutorial. Additional engines require significant code changes to link SD data to voice.cc - the complete integration is beyond this tutorial scope.