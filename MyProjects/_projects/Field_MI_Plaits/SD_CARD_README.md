# SD Card Data for Plaits Extended Engines

This module enables additional Plaits synthesis engines by loading data from SD card.

## Required SD Card Files

Create a folder `/plaits/` on your SD card and add these binary files:

| File | Size | Source |
|------|------|--------|
| `wavetable.bin` | 49,920 bytes | Extract from Plaits `wav_integrated_waves` |
| `table_fold.bin` | 1,032 bytes | Extract from Plaits `lut_fold` |
| `table_fold2.bin` | 1,032 bytes | Extract from Plaits `lut_fold_2` |

## How to Extract Data

1. **From Plaits resources.cc** (in PlaitsPatchInit):
   - Line 1273+: `wav_integrated_waves[]` (49,920 bytes of int16_t)
   - Line ~65: `lut_fold[]` (516 values of float = 1,032 bytes as int16_t)
   - Line ~73: `lut_fold_2[]` (516 values)

2. **Python extraction script** (run on desktop):
```python
# Extract from Plaits resources.cc
import re

with open('plaits/resources.cc', 'r') as f:
    content = f.read()

# Find wav_integrated_waves array
match = re.search(r'wav_integrated_waves\[\] = \{(.*?)\};', content, re.DOTALL)
if match:
    data = [int(x.strip()) for x in match.group(1).split(',') if x.strip()]
    with open('wavetable.bin', 'wb') as f:
        f.write(bytes(int16_t(x) for x in data[:49920 // 2]))
```

## Usage in Field_MI_Plaits

```cpp
#include "sd_card_data.h"

int main() {
    // Initialize hardware
    hw.Init();
    
    // Load extra engine data from SD card BEFORE voice.Init()
    bool sd_loaded = PlaitsLoadSdData();
    
    // Initialize audio/voice
    voice.Init();
    
    // If SD loaded, optionally enable extra engines
    if (sd_loaded) {
        // Extra engines can now access wavetable data
    }
    
    // ... rest of main loop
}
```

## Engine Activation

After SD data is loaded, the engines that need external data become available:

| Engine | Data Required | Slots |
|--------|--------------|-------|
| Wavetable | wavetable.bin | A6 |
| Chord | wavetable.bin | A7 |
| Waveshaping | table_fold.bin, table_fold2.bin | A2 |

## File Location in Project

- Source: `Field_MI_Plaits/sd_card_data.cpp`
- Header: `Field_MI_Plaits/sd_card_data.h`

## Build Notes

1. Add to Makefile:
```makefile
CPP_SOURCES += sd_card_data.cpp
C_INCLUDES += -I$(LIBDAISY_DIR)/Middlewares/Third_Party/FatFs/src
```

2. Ensure FatFS is included in libDaisy

## Status Display

The OLED can show SD status:
```cpp
// In your display update
Display()->WriteString(PlaitsSdStatus(), font, true);
```

## Limitations

- SD card must be present at boot
- If SD load fails, embedded defaults are used (only 9 engines work)
- Data is loaded into RAM (~100KB extra RAM used)