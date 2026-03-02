# Daisy Field FM-Granular Step Sequencer - Debug System Documentation

## Overview

This document describes the comprehensive debug system added to the Daisy Field FM-Granular Step Sequencer project. The debug system provides real-time monitoring of synthesis parameters, step sequencer state, envelope processing, audio levels, and user interface interactions without interfering with real-time audio performance.

## Debug System Features

### 1. Debug Levels

The system supports four debug levels to control verbosity:

- **DEBUG_NONE (0)**: No debug output
- **DEBUG_MINIMAL (1)**: Essential runtime status only
- **DEBUG_NORMAL (2)**: Standard debug output (default)
- **DEBUG_VERBOSE (3)**: Detailed parameter monitoring

### 2. Debug Subsystems

Debug messages are categorized by subsystem:

- **FM**: FM synthesis parameters (ratio, index, frequency)
- **GRANULAR**: Granular oscillator parameters (formant, shape, grain size)
- **SEQ**: Step sequencer state (triggers, timing, advancement)
- **ENV**: Envelope processing (attack/decay, velocity)
- **UI**: User interface interactions (knobs, buttons, modes)
- **AUDIO**: Audio levels and signal monitoring
- **PERF**: Performance monitoring and system statistics

### 3. Message Format

Debug messages follow this format:
```
[LEVEL] [SUBSYSTEM] STEP_XX: message content
```

Example messages:
```
[NORM] [SEQ] STEP_03: STEP TRIGGERED: Step 3, Freq=196.00 Hz, Vel=0.80, Gate=0.50
[VERB] [FM] STEP_03: FM Parameters - Ratio: 1.00, Index: 5.50
[MIN] [UI]: MODE CHANGE: PLAY -> RECORD
```

## Key Debug Features

### FM Synthesis Monitoring
- **FM Parameters**: Real-time ratio and index changes
- **Voice Allocation**: Track which voice gets which frequency
- **Frequency Changes**: Monitor frequency assignments per step

```cpp
DEBUG_FM_VERBOSE("Voice 1: Freq=%.2f Hz", step.frequency);
DEBUG_FM_NORMAL("FM Parameters - Ratio: %.2f, Index: %.2f", fm_ratio, fm_index);
```

### Granular Oscillator Monitoring
- **Formant Frequency**: Track formant parameter changes
- **Grain Shape**: Monitor granular oscillator shape parameter
- **Parameter Updates**: Real-time granular synthesis parameter updates

```cpp
DEBUG_GRANULAR_VERBOSE("Granular Parameters - Formant: %.0f Hz, Shape: %.2f", 
                      formant, grain_shape);
```

### Step Sequencer State Monitoring
- **Step Triggers**: Monitor when steps are triggered
- **Step Advancement**: Track sequence progression
- **Voice Allocation**: Monitor voice assignment and release
- **Tempo Changes**: Real-time BPM and timing updates

```cpp
DEBUG_SEQ_NORMAL("STEP TRIGGERED: Step %d, Freq=%.2f Hz, Vel=%.2f, Gate=%.2f", 
                step_index, step.frequency, step.velocity, step.gate_length);
```

### Envelope Processing Monitoring
- **Attack/Decay Timing**: Monitor envelope timing parameters
- **Velocity Scaling**: Track velocity application
- **Gate Control**: Monitor gate opening/closing
- **Envelope Levels**: Real-time envelope output levels

```cpp
DEBUG_ENV_NORMAL("ENV TRIGGER: Velocity=%.2f", step.velocity);
DEBUG_ENV_VERBOSE("ENV Levels: V1=%.3f, V2=%.3f (vel=%.2f)", 
                 env1_out, env2_out, velocity);
```

### User Interface Interaction Monitoring
- **Knob Changes**: Detect and report significant parameter changes
- **Key Presses**: Monitor button/key interactions
- **Mode Switching**: Track changes between PLAY/RECORD/EDIT modes
- **Parameter Change Detection**: Report old vs new values

```cpp
DEBUG_UI_NORMAL("PARAM CHANGE: FM Index: 2.000 -> 5.500 (via Knob2)");
DEBUG_UI_NORMAL("MODE CHANGE: PLAY -> RECORD");
```

### Audio Level and Performance Monitoring
- **Peak Level Tracking**: Monitor audio output levels
- **Clipping Detection**: Alert when audio clips (>0.95)
- **Dropout Detection**: Identify audio dropouts during active playback
- **Performance Statistics**: Callback rates and message counts

```cpp
DEBUG_AUDIO_NORMAL("CLIPPING DETECTED - Peak: 0.967");
DEBUG_AUDIO_MINIMAL("AUDIO DROPOUT - Very low level: 0.000123");
DEBUG_PERF_NORMAL("Callbacks/sec: 1000, Total messages: 150");
```

## Real-Time Parameter Monitoring

### Parameter Change Detection
The system automatically detects significant parameter changes (>0.01) and reports them with old/new values:

```cpp
void Debug_ReportParameterChange(const char* param_name, float old_val, float new_val, const char* source) {
    float change = fabsf(new_val - old_val);
    if (change > 0.01f) { // Only report significant changes
        DEBUG_UI_NORMAL("PARAM CHANGE: %s: %.3f -> %.3f (via %s)", 
                       param_name, old_val, new_val, source);
    }
}
```

### Voice Allocation Tracking
Monitors when voices are allocated and released:

```cpp
void Debug_ReportVoiceAllocation(int voice_id, float frequency, bool allocated) {
    if (allocated) {
        DEBUG_SEQ_VERBOSE("VOICE ALLOCATED: Voice %d -> Freq: %.2f Hz", voice_id, frequency);
    } else {
        DEBUG_SEQ_VERBOSE("VOICE RELEASED: Voice %d", voice_id);
    }
}
```

## Performance Considerations

### Rate Limiting
To prevent serial buffer overflow during audio processing:
- **Minimum Interval**: 10ms between debug messages
- **Smart Filtering**: Only reports significant changes
- **Performance Monitoring**: Tracks debug message count

```cpp
static uint32_t g_last_debug_time = 0;
static const uint32_t DEBUG_MIN_INTERVAL_MS = 10;

void Debug_Print(DebugLevel level, DebugSubsystem subsystem, const char* subsystem_name, 
                const char* format, ...) {
    uint32_t current_time = System::GetNow();
    if (current_time - g_last_debug_time < DEBUG_MIN_INTERVAL_MS) return;
    // ... rest of function
}
```

### Audio-Safe Debugging
- **Non-Blocking**: Debug calls don't block audio processing
- **Minimal Overhead**: Efficient message formatting and filtering
- **Real-Time Safe**: Designed for embedded real-time systems

## Usage Instructions

### Building with Debug System

1. **Using the Debug Makefile**:
```bash
# Build debug version
make -f Makefile_debug

# Flash to Daisy Field
make -f Makefile_debug flash
```

2. **Setting Debug Level**:
```cpp
// In main() function, change this line:
static DebugLevel g_debug_level = DEBUG_NORMAL;  // Change to desired level

// Options:
DEBUG_NONE      // No output
DEBUG_MINIMAL   // Essential info only
DEBUG_NORMAL    // Standard output (recommended)
DEBUG_VERBOSE   // Maximum detail
```

3. **Enabling/Disabling Subsystems**:
```cpp
// Enable all subsystems (default)
static uint8_t g_debug_subsystems = DEBUG_ALL;

// Enable only specific subsystems:
static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_FM | DEBUG_ENV;

// Enable individual subsystems:
DEBUG_FM        // FM synthesis only
DEBUG_GRANULAR  // Granular oscillator only
DEBUG_SEQ       // Sequencer only
DEBUG_ENV       // Envelope only
DEBUG_UI        // UI interactions only
DEBUG_AUDIO     // Audio levels only
DEBUG_PERF      // Performance only
```

### Serial Monitor Setup

1. **Connect to Daisy Field**:
   - Use Daisy Field's USB serial connection
   - Baud rate: 115200
   - Data bits: 8
   - Stop bits: 1
   - Parity: None

2. **Example Serial Output**:
```
[MIN] [SEQ]: === DAISY FIELD FM-GRANULAR SEQUENCER STARTING ===
[NORM] [SEQ]: Debug Level: NORMAL, Subsystems: ALL
[NORM] [AUDIO]: Hardware initialized - Sample Rate: 48000 Hz, Block Size: 48
[NORM] [SEQ]: SEQUENCE INITIALIZED: 8-step C major scale, BPM: 120.0
[NORM] [SEQ]: Clock initialized - BPM: 120.0
[NORM] [FM]: Voice 1 initialized - FM: C3, Env: 5ms/200ms
[NORM] [FM]: Voice 2 initialized - FM: C3, Env: 5ms/200ms
[NORM] [AUDIO]: Effects initialized - Overdrive, Reverb (fb=0.6, lp=8kHz)
[MIN] [SEQ]: Audio system started - Ready for operation
```

### Debug Message Categories

#### Step Sequencer Messages
```
[NORM] [SEQ] STEP_00: STEP TRIGGERED: Step 0, Freq=130.81 Hz, Vel=0.80, Gate=0.50
[VERB] [SEQ] STEP_01: STEP ADVANCE: 0 -> 1
[VERB] [SEQ] STEP_01: VOICE ALLOCATED: Voice 1 -> Freq: 146.83 Hz
```

#### FM Synthesis Messages
```
[VERB] [FM] STEP_01: Voice 1: Freq=146.83 Hz
[VERB] [FM] STEP_01: Voice 2: Freq=146.83 Hz (Ratio: 1.0)
[NORM] [FM] STEP_01: FM Parameters - Ratio: 1.00, Index: 5.50
[VERB] [FM] STEP_01: Filter Parameters - Cutoff: 5000.0 Hz, Resonance: 0.30
```

#### Granular Synthesis Messages
```
[VERB] [GRANULAR] STEP_01: Grainlet Freqs set to 146.83 Hz
[NORM] [GRANULAR] STEP_01: Granular Parameters - Formant: 2500.0 Hz, Shape: 0.50
```

#### Envelope Messages
```
[NORM] [ENV] STEP_01: ENV TRIGGER: Velocity=0.80
[VERB] [ENV] STEP_01: ENV TRIGGERED: Both voices
[VERB] [ENV] STEP_01: ENV Levels: V1=0.642, V2=0.642 (vel=0.80)
[VERB] [ENV] STEP_01: GATE CLOSED: Step 1, Duration: 0.50
```

#### UI Interaction Messages
```
[NORM] [UI]: PARAM CHANGE: FM Index: 2.000 -> 5.500 (via Knob2)
[NORM] [UI]: MODE CHANGE: PLAY -> RECORD
[NORM] [UI]: STEP 0 RECORDED: Freq=130.81 Hz
[VERB] [UI]: EDIT MODE: Selected step 3 for editing
```

#### Audio Level Messages
```
[NORM] [AUDIO]: CLIPPING DETECTED - Peak: 0.967
[MIN] [AUDIO]: AUDIO DROPOUT - Very low level: 0.000123
```

#### Performance Messages
```
[NORM] [PERF]: Callbacks/sec: 1000, Total messages: 2450
[VERB] [PERF]: Audio buffer processed - Size: 48 samples
```

## Troubleshooting

### Common Issues

1. **Too Much Output**: Reduce debug level or disable subsystems
   ```cpp
   static DebugLevel g_debug_level = DEBUG_MINIMAL;
   ```

2. **Performance Impact**: Use DEBUG_MINIMAL for performance-critical testing
   ```cpp
   static DebugLevel g_debug_level = DEBUG_MINIMAL;
   ```

3. **Buffer Overflow**: The system includes rate limiting, but reduce verbosity if issues persist
   ```cpp
   static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_AUDIO; // Essential only
   ```

### Debug Configuration for Different Scenarios

#### Development/Debugging
```cpp
static DebugLevel g_debug_level = DEBUG_VERBOSE;
static uint8_t g_debug_subsystems = DEBUG_ALL;
```

#### Performance Testing
```cpp
static DebugLevel g_debug_level = DEBUG_MINIMAL;
static uint8_t g_debug_subsystems = DEBUG_SEQ | DEBUG_PERF;
```

#### Audio Quality Testing
```cpp
static DebugLevel g_debug_level = DEBUG_NORMAL;
static uint8_t g_debug_subsystems = DEBUG_AUDIO | DEBUG_ENV;
```

#### UI Development
```cpp
static DebugLevel g_debug_level = DEBUG_NORMAL;
static uint8_t g_debug_subsystems = DEBUG_UI | DEBUG_SEQ;
```

## System Requirements

- **Daisy Field Hardware**
- **libDaisy Library** (latest version)
- **DaisySP Library** (latest version)
- **Serial Terminal Software** (115200 baud)
- **ARM GCC Toolchain** (for compilation)

## Integration Notes

The debug system is designed to be:
- **Non-Intrusive**: Doesn't affect audio performance when disabled
- **Configurable**: Easy to enable/disable specific subsystems
- **Real-Time Safe**: Uses rate limiting and efficient formatting
- **Comprehensive**: Covers all major synthesis and sequencer components
- **Maintainable**: Clear macro structure for easy expansion

This debug system provides comprehensive real-time monitoring capabilities while maintaining the real-time performance requirements of the Daisy Field hardware platform.