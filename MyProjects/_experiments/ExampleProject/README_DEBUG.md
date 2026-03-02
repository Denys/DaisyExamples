# Daisy Field FM-Granular Step Sequencer - Debug System Implementation

## Project Summary

This project adds comprehensive serial monitor debug messages to the Daisy Field FM-Granular Step Sequencer for real-time monitoring of all synthesis parameters, sequencer state, envelope processing, audio levels, and user interface interactions.

## What Was Implemented

### 1. Comprehensive Debug System
- **4 Debug Levels**: NONE, MINIMAL, NORMAL, VERBOSE
- **7 Subsystems**: FM, GRANULAR, SEQ, ENV, UI, AUDIO, PERF
- **Rate Limiting**: Prevents serial buffer overflow (10ms minimum interval)
- **Real-time Safe**: Non-blocking debug output

### 2. FM Synthesis Monitoring
- FM ratio and index parameter changes
- Voice allocation and frequency assignment
- Carrier/modulator frequency tracking
- Real-time parameter change detection

### 3. Granular Oscillator Monitoring
- Formant frequency changes
- Grain shape parameter updates
- Granular oscillator frequency settings
- Grain size and density tracking

### 4. Step Sequencer Debug Output
- Step trigger events with full parameter details
- Step advancement timing and progression
- Voice allocation and release tracking
- Tempo sync status and BPM changes
- Gate length and timing monitoring

### 5. Envelope Processing Messages
- Attack/decay timing parameters
- Velocity scaling and envelope levels
- Gate open/close events
- Trigger events for both voices

### 6. User Interface Interaction Debug
- Knob change detection with old/new values
- Key press/release events
- Mode switching (PLAY/RECORD/EDIT)
- Step selection and parameter editing

### 7. Audio Level and Performance Monitoring
- Peak level tracking with clipping detection
- Audio dropout identification
- Callback rate monitoring
- Performance statistics reporting

## Files Created

1. **ExampleProject_debug.cpp**: Complete debug-enabled version of the sequencer
2. **Makefile_debug**: Build configuration for debug version
3. **DEBUG_SYSTEM_DOCUMENTATION.md**: Comprehensive usage documentation
4. **README_DEBUG.md**: This summary document

## Debug Message Format

All debug messages follow this standardized format:
```
[LEVEL] [SUBSYSTEM] STEP_XX: message content
```

### Example Output:
```
[NORM] [SEQ] STEP_03: STEP TRIGGERED: Step 3, Freq=196.00 Hz, Vel=0.80, Gate=0.50
[VERB] [FM] STEP_03: FM Parameters - Ratio: 1.00, Index: 5.50
[VERB] [GRANULAR] STEP_03: Granular Parameters - Formant: 2500.0 Hz, Shape: 0.50
[NORM] [ENV] STEP_03: ENV TRIGGER: Velocity=0.80
[NORM] [UI]: PARAM CHANGE: FM Index: 2.000 -> 5.500 (via Knob2)
[NORM] [PERF]: Callbacks/sec: 1000, Total messages: 2450
```

## Key Features

### Real-time Parameter Monitoring
- Automatic detection of significant parameter changes (>0.01)
- Reports old value, new value, and source (knob/button)
- Tracks parameter changes across all 8 knobs in different modes

### Voice Allocation Tracking
- Monitors when voices are allocated to frequencies
- Tracks voice release and reuse
- Reports frequency assignment for each voice

### Audio Performance Monitoring
- Peak level detection with clipping alerts (>0.95)
- Dropout detection for troubleshooting
- Performance statistics every second

### Comprehensive Coverage
- **FM**: Ratio, index, frequency, voice allocation
- **GRANULAR**: Formant, shape, grain parameters
- **SEQ**: Step triggers, timing, advancement, tempo
- **ENV**: Attack/decay, velocity, gate control
- **UI**: Knobs, keys, modes, parameter changes
- **AUDIO**: Levels, clipping, dropouts
- **PERF**: Callbacks, message rates, system stats

## Usage Instructions

### Quick Start
1. Use `ExampleProject_debug.cpp` instead of `ExampleProject.cpp`
2. Build with `Makefile_debug`
3. Connect serial monitor at 115200 baud
4. Debug output will appear automatically

### Configuration
```cpp
// Debug level (NONE, MINIMAL, NORMAL, VERBOSE)
static DebugLevel g_debug_level = DEBUG_NORMAL;

// Enable/disable subsystems
static uint8_t g_debug_subsystems = DEBUG_ALL;  // All enabled
// Or: DEBUG_SEQ | DEBUG_FM | DEBUG_ENV  // Specific ones only
```

### Build Commands
```bash
# Build debug version
make -f Makefile_debug

# Flash to Daisy Field
make -f Makefile_debug flash
```

## Performance Considerations

### Buffer Overflow Prevention
- 10ms minimum interval between messages
- Smart filtering (only significant changes)
- Efficient string formatting
- Non-blocking debug calls

### Audio Processing Safety
- Debug calls don't block audio callback
- Minimal computational overhead
- Real-time system compatible
- No impact on audio quality when enabled

## Debug Levels Explained

### DEBUG_NONE
No debug output - maximum performance

### DEBUG_MINIMAL
Essential information only:
- System startup
- Mode changes
- Critical errors (clipping, dropouts)
- Performance summaries

### DEBUG_NORMAL (Recommended)
Standard debug output:
- All minimal messages
- Step triggers
- Parameter changes
- UI interactions
- Audio warnings

### DEBUG_VERBOSE
Maximum detail:
- All normal messages
- Every parameter update
- Detailed envelope levels
- Voice allocation events
- System timing details

## Subsystem Control

Individual subsystems can be enabled/disabled:

```cpp
// Enable only specific subsystems
static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_AUDIO;

// Available flags:
DEBUG_FM        // FM synthesis
DEBUG_GRANULAR  // Granular oscillator
DEBUG_SEQ       // Step sequencer
DEBUG_ENV       // Envelope processing
DEBUG_UI        // User interface
DEBUG_AUDIO     // Audio levels
DEBUG_PERF      // Performance monitoring
```

## Troubleshooting

### Too Much Output?
```cpp
static DebugLevel g_debug_level = DEBUG_MINIMAL;
```

### Performance Issues?
```cpp
static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_PERF;
```

### Need Specific Info?
```cpp
// For audio troubleshooting
static uint8_t g_debug_subsystems = DEBUG_AUDIO | DEBUG_ENV;

// For sequencer debugging
static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_FM;

// For UI development
static uint8_t g_debug_subsystems = DEBUG_UI | DEBUG_SEQ;
```

## Technical Implementation

### Rate Limiting System
```cpp
static uint32_t g_last_debug_time = 0;
static const uint32_t DEBUG_MIN_INTERVAL_MS = 10;

void Debug_Print(...) {
    uint32_t current_time = System::GetNow();
    if (current_time - g_last_debug_time < DEBUG_MIN_INTERVAL_MS) return;
    // Process debug message
}
```

### Parameter Change Detection
```cpp
void Debug_ReportParameterChange(const char* param_name, float old_val, float new_val, const char* source) {
    float change = fabsf(new_val - old_val);
    if (change > 0.01f) { // Only significant changes
        DEBUG_UI_NORMAL("PARAM CHANGE: %s: %.3f -> %.3f (via %s)", 
                       param_name, old_val, new_val, source);
    }
}
```

### Audio Level Monitoring
```cpp
void Debug_ReportAudioLevels(float left_level, float right_level) {
    float peak = fmaxf(fabsf(left_level), fabsf(right_level));
    if (peak > 0.95f) {
        DEBUG_AUDIO_NORMAL("CLIPPING DETECTED - Peak: %.3f", peak);
    }
    if (peak < 0.001f && sequencer_running && step_triggered) {
        DEBUG_AUDIO_MINIMAL("AUDIO DROPOUT - Very low level: %.6f", peak);
    }
}
```

## Benefits

1. **Real-time Monitoring**: See exactly what's happening during playback
2. **Troubleshooting**: Identify audio issues, parameter conflicts, timing problems
3. **Development**: Understand system behavior during development
4. **Performance**: Monitor system load and audio quality
5. **User Experience**: Debug UI interactions and mode switching
6. **Synthesis Learning**: Understand FM and granular synthesis parameter effects

This debug system provides comprehensive visibility into the FM-Granular Step Sequencer's operation while maintaining real-time performance and audio quality.