# Daisy Field Serial Debugging Strategy

> **Reference**: [daisy.audio Serial Printing Tutorial](https://daisy.audio/tutorials/_a2_Getting-Started-Serial-Printing/)

## Quick Start

### 1. Enable Serial Logging
```cpp
// In main(), after hw.Init():
hw.StartLog();        // Non-blocking, program continues immediately
// OR
hw.StartLog(true);    // Blocks until USB serial connection established
```

### 2. Serial Monitor Tools
- **VS Code**: Install "Serial Port Helper" extension (set view: "string" not "hex")
- **Arduino IDE**: Built-in Serial Monitor works great
- **PuTTY/Tera Term**: Any serial terminal works

### 3. Print Debug Messages
```cpp
hw.PrintLine("Hello World!");                    // Simple string
hw.PrintLine("Value: %d", int_value);            // Integer
hw.PrintLine("Key %d pressed", key_index);       // With variables
```

---

## Debug Patterns

### Pattern 1: Startup Checkpoint
Use when: You're not sure if the program crashes during initialization.

```cpp
hw.Init();
hw.StartLog(true);  // Wait for connection

hw.PrintLine("=== STARTUP ===");
hw.PrintLine("1. Hardware initialized");

// Your init code here...
modal[0].Init(sr);
hw.PrintLine("2. Modal voices initialized");

keyLeds.Init(&hw);
hw.PrintLine("3. LEDs initialized");

hw.StartAudio(AudioCallback);
hw.PrintLine("4. Audio started - entering main loop");
```

### Pattern 2: Event Triggered Debug
Use when: Crash happens on specific action (like pressing a key).

```cpp
if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
{
    hw.PrintLine("B-key %d pressed", i);
    
    // More detailed debug
    hw.PrintLine("  strike_type = %d", strike_type);
    hw.PrintLine("  active modes = %d", active_count);
    
    // Your actual code...
    keyLeds.SetB(i, true);
    hw.PrintLine("  LED set OK");
    
    // Before the suspected crash point
    hw.PrintLine("  About to trigger modes...");
    for(int m = 0; m < kNumModes; m++)
    {
        if(mode_active[m])
        {
            hw.PrintLine("    Triggering mode %d", m);
            modal[m].Trig();
        }
    }
    hw.PrintLine("  All modes triggered OK");
}
```

### Pattern 3: Periodic Loop Status
Use when: System crashes randomly or after some time.

```cpp
static int loop_count = 0;
while(1)
{
    hw.ProcessAllControls();
    
    // Print status every 1000 loops (~16 seconds at 60Hz)
    loop_count++;
    if(loop_count % 1000 == 0)
    {
        hw.PrintLine("Loop %d OK - modes=%d strike=%d", 
                     loop_count, active_count, strike_type);
    }
    
    // Your code...
    System::Delay(16);
}
```

### Pattern 4: Audio Callback Debug
**⚠️ WARNING**: Avoid printing in AudioCallback! It can cause buffer underruns.

```cpp
// Instead, use a flag system:
volatile bool debug_trigger = false;
volatile int  debug_mode    = -1;

void AudioCallback(...) 
{
    // Set flag, don't print here!
    if(some_condition)
    {
        debug_trigger = true;
        debug_mode = current_mode;
    }
}

// In main loop, check and print:
if(debug_trigger)
{
    hw.PrintLine("Audio debug: mode=%d", debug_mode);
    debug_trigger = false;
}
```

---

## Floating Point Printing

### Option A: Enable %f Support (adds ~3KB to binary)
```makefile
# In your Makefile:
LDFLAGS += -u _printf_float
```
Then use:
```cpp
hw.PrintLine("Knob value: %f", knob_val);
```

### Option B: Use FLT_FMT Macros (no size cost)
```cpp
hw.PrintLine("Value: " FLT_FMT3, FLT_VAR3(my_float));  // 3 decimals
hw.PrintLine("Value: " FLT_FMT(6), FLT_VAR(6, my_float));  // 6 decimals
```

### Option C: Integer Scaling (simplest)
```cpp
// Convert 0.0-1.0 to 0-100
int percent = (int)(knob_val * 100);
hw.PrintLine("Knob: %d%%", percent);
```

---

## Debug Build Configuration

### Makefile Option
```makefile
# Add debug flag for conditional compilation
ifeq ($(DEBUG), 1)
CPPFLAGS += -DDEBUG_SERIAL
LDFLAGS += -u _printf_float
endif
```

### In Code
```cpp
#ifdef DEBUG_SERIAL
#define DBG_PRINT(fmt, ...) hw.PrintLine(fmt, ##__VA_ARGS__)
#else
#define DBG_PRINT(fmt, ...)  // No-op in release
#endif

// Usage:
DBG_PRINT("Key %d pressed", i);  // Only compiles in debug
```

Build with: `make DEBUG=1`

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No output | Check USB cable, ensure `hw.StartLog()` called |
| Garbled text | Set serial monitor to 115200 baud |
| Missing early prints | Use `hw.StartLog(true)` to wait for connection |
| Float prints as "?" | Add `LDFLAGS += -u _printf_float` or use FLT_FMT |
| System crashes with prints | Don't print in AudioCallback! |

---

## Integration with DAISY_BUGS.md

When debugging a crash, add serial debug points and document findings:

```markdown
### Analysis

**Step 4: Serial Debug Output**
```
=== STARTUP ===
1. Hardware initialized
2. Modal voices initialized
...
B-key 3 pressed
  strike_type = 3
  About to trigger modes...
    Triggering mode 0
    Triggering mode 1
<< CRASH - no more output >>
```

**Conclusion**: Crash occurs when triggering mode 1 after mode 0.
```

---

## VS Code Hardware Debugging (ST-Link)

> **Reference**: [daisy.audio C++ Dev Environment](https://daisy.audio/tutorials/cpp-dev-env/)

### Prerequisites

1. **Install Cortex Debug Extension**:
   - VS Code: `View > Extensions`
   - Search "Cortex Debug" by marus25
   - Install

2. **Connect ST-Link V3 Mini**:
   - Connect debug probe to Daisy Field JTAG header
   - Red stripe facing correct direction
   - Power device normally

3. **Windows Only**: Set VS Code terminal to Git Bash:
   - Terminal dropdown → `Select Default Profile` → `Git Bash`

### Debug Workflow

| Action | Keyboard | Description |
|--------|----------|-------------|
| Start Debug | **F5** | Builds, flashes, halts at entry |
| Continue | Play button | Resume execution |
| Step Over | **F10** | Execute line, skip into functions |
| Step Into | **F11** | Execute line, enter functions |
| Step Out | **Shift+F11** | Exit current function |
| Stop Debug | **Shift+F5** | End session |

### Setting Breakpoints

```cpp
// Click in the gutter (left of line numbers) to set breakpoint
if(hw.KeyboardRisingEdge(kKeyBIndices[i]))  // <-- Set breakpoint here
{
    strike_type = i;  // Execution pauses, inspect variables
}
```

### VS Code Debug Panel

When paused at breakpoint:
- **Variables**: View local/global variable values
- **Watch**: Add expressions to monitor
- **Call Stack**: See function call hierarchy
- **Breakpoints**: Manage all breakpoints

### VS Code Tasks

| Task | Command Palette (Ctrl+P) |
|------|--------------------------|
| Build libraries | `task build_all` |
| Build + Flash | `task build_and_program` |

### When to Use Hardware Debug vs Serial

| Use Case | Recommended Method |
|----------|-------------------|
| Crash location unknown | **Serial** (checkpoints) |
| Need to inspect variables | **Hardware Debug** |
| Crash in audio callback | **Hardware Debug** (can pause safely) |
| General flow tracing | **Serial** |
| Complex state inspection | **Hardware Debug** |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No output | Check USB cable, ensure `hw.StartLog()` called |
| Garbled text | Set serial monitor to 115200 baud |
| Missing early prints | Use `hw.StartLog(true)` to wait for connection |
| Float prints as "?" | Add `LDFLAGS += -u _printf_float` or use FLT_FMT |
| System crashes with prints | Don't print in AudioCallback! |
| `mkdir build` error (Windows) | Change terminal to Git Bash |
| F5 debug not working | Check ST-Link connection, install Cortex Debug |
| Can't find debug probe | Run Zadig to reset USB driver (Windows) |

---

## Quality Assurance Ecosystem

This document is part of an interconnected quality assurance system:

| Document | Purpose | When to Use |
|----------|---------|-------------|
| [DAISY_TUTORIALS_KNOWLEDGE.md](DAISY_TUTORIALS_KNOWLEDGE.md) | Official API reference | Looking up GPIO/Audio/ADC usage |
| [DAISY_DEVELOPMENT_STANDARDS.md](DAISY_DEVELOPMENT_STANDARDS.md) | Workflow patterns | Starting a new project |
| [DAISY_TECHNICAL_REPORT.md](DAISY_TECHNICAL_REPORT.md) | Complete process documentation | Deep reference, onboarding |
| [DAISY_BUGS.md](DAISY_BUGS.md) | Bug tracking, investigation methodology | Documenting issues, searching past fixes |

**This document's role**: Provides debugging techniques when things aren't working. Use the checkpoints and patterns here after confirming your code follows the standards in DEVELOPMENT_STANDARDS.md.

---

**Document Version**: 1.1
**Last Updated**: 2026-02-08

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.1 | 2026-02-08 | Added VS Code hardware debugging section (ST-Link), debug build configuration |
| 1.0 | 2026-02-08 | Initial version: 4 debug patterns, float printing, troubleshooting table |
