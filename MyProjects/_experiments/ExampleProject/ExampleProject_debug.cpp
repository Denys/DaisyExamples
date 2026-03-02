// Step Sequencer - Daisy Field with Comprehensive Debug System
// 8-step sequencer with FM-Granular synthesis
// Date: 2025-12-13

#include "daisy_field.h"
#include "daisysp.h"
#include "hid/logger.h"

using namespace daisy;
using namespace daisysp;
using Logger = daisy::Logger<daisy::LOGGER_INTERNAL>;

// ========================================
// DEBUG SYSTEM DEFINITIONS
// ========================================

// Debug levels
enum DebugLevel {
    DEBUG_NONE = 0,      // No debug output
    DEBUG_MINIMAL = 1,   // Essential runtime status only
    DEBUG_NORMAL = 2,    // Standard debug output
    DEBUG_VERBOSE = 3    // Detailed parameter monitoring
};

// Debug subsystems
enum DebugSubsystem {
    DEBUG_FM = (1 << 0),
    DEBUG_GRANULAR = (1 << 1),
    DEBUG_SEQ = (1 << 2),
    DEBUG_ENV = (1 << 3),
    DEBUG_UI = (1 << 4),
    DEBUG_AUDIO = (1 << 5),
    DEBUG_PERF = (1 << 6),
    DEBUG_ALL = 0xFF
};

// Global debug configuration
static DebugLevel g_debug_level = DEBUG_NORMAL;
static uint8_t g_debug_subsystems = DEBUG_ALL;
static bool g_debug_enabled = true;
static uint32_t g_debug_message_count = 0;
static uint32_t g_debug_step_counter = 0;

// Debug rate limiting
static uint32_t g_last_debug_time = 0;
static const uint32_t DEBUG_MIN_INTERVAL_MS = 10; // Minimum 10ms between messages

// ========================================
// DEBUG MACROS AND FUNCTIONS
// ========================================

#define DEBUG_FORMAT "[%s] [%s] STEP_%02lu: "
#define DEBUG_FORMAT_NO_STEP "[%s] [%s]: "

// Debug print function with rate limiting
void Debug_Print(DebugLevel level, DebugSubsystem subsystem, const char* subsystem_name, 
                const char* format, ...) {
    if (!g_debug_enabled || level > g_debug_level) return;
    if (!(subsystem & g_debug_subsystems)) return;
    
    uint32_t current_time = System::GetNow();
    if (current_time - g_last_debug_time < DEBUG_MIN_INTERVAL_MS) return;
    
    g_last_debug_time = current_time;
    
    char buffer[256];
    size_t offset = 0;
    
    // Format prefix
    const char* level_str[] = {"NONE", "MIN", "NORM", "VERB"};
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, 
                      DEBUG_FORMAT, level_str[level], subsystem_name, g_debug_step_counter);
    
    // Format message
    va_list args;
    va_start(args, format);
    offset += vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);
    
    // Add newline and ensure null termination
    if (offset < sizeof(buffer) - 1) {
        buffer[offset++] = '\n';
        buffer[offset] = '\0';
    }
    
    // Output to serial
    daisy::Logger<daisy::LOGGER_INTERNAL>::Print(buffer);
    
    g_debug_message_count++;
}

// Debug macros for different verbosity levels
#define DEBUG_FM_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_FM, "FM", fmt, ##__VA_ARGS__)
#define DEBUG_FM_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_FM, "FM", fmt, ##__VA_ARGS__)
#define DEBUG_FM_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_FM, "FM", fmt, ##__VA_ARGS__)

#define DEBUG_GRANULAR_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_GRANULAR, "GRANULAR", fmt, ##__VA_ARGS__)
#define DEBUG_GRANULAR_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_GRANULAR, "GRANULAR", fmt, ##__VA_ARGS__)
#define DEBUG_GRANULAR_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_GRANULAR, "GRANULAR", fmt, ##__VA_ARGS__)

#define DEBUG_SEQ_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_SEQ, "SEQ", fmt, ##__VA_ARGS__)
#define DEBUG_SEQ_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_SEQ, "SEQ", fmt, ##__VA_ARGS__)
#define DEBUG_SEQ_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_SEQ, "SEQ", fmt, ##__VA_ARGS__)

#define DEBUG_ENV_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_ENV, "ENV", fmt, ##__VA_ARGS__)
#define DEBUG_ENV_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_ENV, "ENV", fmt, ##__VA_ARGS__)
#define DEBUG_ENV_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_ENV, "ENV", fmt, ##__VA_ARGS__)

#define DEBUG_UI_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_UI, "UI", fmt, ##__VA_ARGS__)
#define DEBUG_UI_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_UI, "UI", fmt, ##__VA_ARGS__)
#define DEBUG_UI_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_UI, "UI", fmt, ##__VA_ARGS__)

#define DEBUG_AUDIO_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_AUDIO, "AUDIO", fmt, ##__VA_ARGS__)
#define DEBUG_AUDIO_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_AUDIO, "AUDIO", fmt, ##__VA_ARGS__)
#define DEBUG_AUDIO_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_AUDIO, "AUDIO", fmt, ##__VA_ARGS__)

#define DEBUG_PERF_VERBOSE(fmt, ...) \
    Debug_Print(DEBUG_VERBOSE, DEBUG_PERF, "PERF", fmt, ##__VA_ARGS__)
#define DEBUG_PERF_NORMAL(fmt, ...) \
    Debug_Print(DEBUG_NORMAL, DEBUG_PERF, "PERF", fmt, ##__VA_ARGS__)
#define DEBUG_PERF_MINIMAL(fmt, ...) \
    Debug_Print(DEBUG_MINIMAL, DEBUG_PERF, "PERF", fmt, ##__VA_ARGS__)

// Hardware
DaisyField hw;

// DSP Modules - Voice 1
Fm2 fm1;
GrainletOscillator grainlet1;
MoogLadder filter1;
AdEnv env1;  // Changed from Adsr to AdEnv

// DSP Modules - Voice 2
Fm2 fm2;
GrainletOscillator grainlet2;
MoogLadder filter2;
AdEnv env2;  // Changed from Adsr to AdEnv

// DSP Modules - Effects Chain
Overdrive overdrive;
ReverbSc DSY_SDRAM_BSS reverb;

// Sequencer modules
Metro clock;
Metro gate_timer;

// Sequencer state
const int NUM_STEPS = 8;
struct Step {
    float frequency;      // Stored frequency for this step
    float velocity;       // 0.0 - 1.0
    bool active;          // Is this step active in sequence?
    float gate_length;    // Gate length as fraction of step (0.1 - 1.0)
};

Step sequence[NUM_STEPS];
int current_step = 0;
bool sequencer_running = false;
bool step_triggered = false;
bool gate_active = false;

// Keyboard state
bool key_state[16] = {false};
bool key_prev[16] = {false};

// Piano-style keyboard layout (same as original)
const float KEY_FREQUENCIES[16] = {
    // Top row (A1-A8): Used for step selection
    130.81f,  // A1 = C3
    146.83f,  // A2 = D3
    164.81f,  // A3 = E3
    174.61f,  // A4 = F3
    196.00f,  // A5 = G3
    220.00f,  // A6 = A3
    246.94f,  // A7 = B3
    261.63f,  // A8 = C4
    
    // Bottom row (B1-B8): Control functions
    138.59f,  // B1 = START/STOP
    155.56f,  // B2 = RESET
    164.81f,  // B3 = RECORD MODE TOGGLE
    185.00f,  // B4 = CLEAR STEP
    207.65f,  // B5 = unused
    233.08f,  // B6 = unused
    0.0f,     // B7 = unused
    0.0f      // B8 = unused
};

// Control modes
enum ControlMode {
    MODE_PLAY = 0,        // Play mode - sequence runs
    MODE_RECORD = 1,      // Record mode - top row programs steps
    MODE_EDIT = 2         // Edit mode - adjust step parameters
};

ControlMode current_mode = MODE_PLAY;
int selected_step = 0;  // For editing (0-7)

// Tempo and timing
float bpm = 120.0f;
float swing_amount = 0.0f;  // 0.0 = straight, 1.0 = max swing

// Audio level monitoring
float peak_audio_level = 0.0f;
float audio_level_history[32] = {0};
int audio_level_index = 0;

// Performance monitoring
uint32_t audio_callback_count = 0;
uint32_t last_perf_report_time = 0;

// ========================================
// DEBUG HELPER FUNCTIONS
// ========================================

void Debug_ReportAudioLevels(float left_level, float right_level) {
    // Update peak level
    float peak = (fabsf(left_level) > fabsf(right_level)) ? fabsf(left_level) : fabsf(right_level);
    peak_audio_level = fmaxf(peak_audio_level * 0.99f, peak);
    
    // Store in history
    audio_level_history[audio_level_index] = peak;
    audio_level_index = (audio_level_index + 1) % 32;
    
    // Report on threshold crossings
    if (peak_audio_level > 0.95f) {
        DEBUG_AUDIO_NORMAL("CLIPPING DETECTED - Peak: %.3f", peak_audio_level);
    }
    
    // Report dropout detection
    if (peak < 0.001f && sequencer_running && step_triggered) {
        DEBUG_AUDIO_MINIMAL("AUDIO DROPOUT - Very low level: %.6f", peak);
    }
}

void Debug_ReportPerformance() {
    uint32_t current_time = System::GetNow();
    if (current_time - last_perf_report_time > 1000) { // Every second
        uint32_t callbacks_per_second = audio_callback_count * 1000 / (current_time - last_perf_report_time);
        
        DEBUG_PERF_NORMAL("Callbacks/sec: %lu, Total messages: %lu", 
                         callbacks_per_second, g_debug_message_count);
        
        audio_callback_count = 0;
        last_perf_report_time = current_time;
    }
}

void Debug_ReportParameterChange(const char* param_name, float old_val, float new_val, const char* source) {
    float change = fabsf(new_val - old_val);
    if (change > 0.01f) { // Only report significant changes
        DEBUG_UI_NORMAL("PARAM CHANGE: %s: %.3f -> %.3f (via %s)", 
                       param_name, old_val, new_val, source);
    }
}

void Debug_ReportVoiceAllocation(int voice_id, float frequency, bool allocated) {
    if (allocated) {
        DEBUG_SEQ_VERBOSE("VOICE ALLOCATED: Voice %d -> Freq: %.2f Hz", voice_id, frequency);
    } else {
        DEBUG_SEQ_VERBOSE("VOICE RELEASED: Voice %d", voice_id);
    }
}

// ========================================
// SEQUENCER FUNCTIONS WITH DEBUG
// ========================================

void InitializeSequence() {
    // Default pattern: C major scale up
    for(int i = 0; i < NUM_STEPS; i++) {
        sequence[i].frequency = KEY_FREQUENCIES[i];  // C3-C4 scale
        sequence[i].velocity = 0.8f;
        sequence[i].active = true;
        sequence[i].gate_length = 0.5f;  // 50% gate length
    }
    
    DEBUG_SEQ_NORMAL("SEQUENCE INITIALIZED: 8-step C major scale, BPM: %.1f", bpm);
}

void UpdateClock() {
    // Calculate step time from BPM (16th notes)
    float step_freq = (bpm / 60.0f) * 4.0f;  // 4 steps per beat at 16th notes
    clock.SetFreq(step_freq);
    
    // Gate timer runs faster to handle gate_length within each step
    gate_timer.SetFreq(step_freq * 10.0f);
    
    DEBUG_SEQ_VERBOSE("CLOCK UPDATED: BPM=%.1f, StepFreq=%.2f Hz, GateFreq=%.2f Hz", 
                     bpm, step_freq, step_freq * 10.0f);
}

void TriggerStep(int step_index) {
    if(step_index < 0 || step_index >= NUM_STEPS) return;
    if(!sequence[step_index].active) {
        DEBUG_SEQ_VERBOSE("STEP %d SKIPPED: inactive", step_index);
        return;
    }
    
    Step& step = sequence[step_index];
    g_debug_step_counter = step_index;
    
    DEBUG_SEQ_NORMAL("STEP TRIGGERED: Step %d, Freq=%.2f Hz, Vel=%.2f, Gate=%.2f", 
                    step_index, step.frequency, step.velocity, step.gate_length);
    
    // Set frequencies for both voices (unison or octave)
    fm1.SetFrequency(step.frequency);
    fm2.SetFrequency(step.frequency * 1.0f);  // Unison (change to 2.0f for octave)
    
    grainlet1.SetFreq(step.frequency);
    grainlet2.SetFreq(step.frequency);
    
    // Debug FM parameters
    DEBUG_FM_VERBOSE("Voice 1: Freq=%.2f Hz", step.frequency);
    DEBUG_FM_VERBOSE("Voice 2: Freq=%.2f Hz (Ratio: %.1f)", step.frequency * 1.0f, 1.0f);
    
    // Debug granular parameters
    DEBUG_GRANULAR_VERBOSE("Grainlet Freqs set to %.2f Hz", step.frequency);
    
    // Trigger envelopes with velocity scaling
    step_triggered = true;
    gate_active = true;
    
    // Debug voice allocation
    Debug_ReportVoiceAllocation(1, step.frequency, true);
    Debug_ReportVoiceAllocation(2, step.frequency * 1.0f, true);
    
    // Debug envelope trigger
    DEBUG_ENV_NORMAL("ENV TRIGGER: Velocity=%.2f", step.velocity);
}

// ========================================
// MAIN AUDIO CALLBACK WITH COMPREHENSIVE DEBUG
// ========================================

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size) {
    audio_callback_count++;
    
    // Process all hardware controls once per audio block
    hw.ProcessAllControls();
    
    // Read knob values based on current mode
    float fm_ratio, fm_index, formant, grain_shape;
    float cutoff, resonance;
    float drive, reverb_feedback;
    
    // static float prev_fm_ratio = -1, prev_fm_index = -1, prev_formant = -1; // Unused
    static float prev_fm_index = -1, prev_formant = -1;
    // static float prev_grain_shape = -1, prev_cutoff = -1, prev_resonance = -1; // Unused
    static float prev_cutoff = -1, prev_resonance = -1;
    static float prev_drive = -1, prev_reverb_feedback = -1;
    
    if(current_mode == MODE_PLAY || current_mode == MODE_RECORD) {
        // Knob 0: Tempo (60-240 BPM)
        float prev_bpm = bpm;
        bpm = 60.0f + hw.knob[0].Value() * 180.0f;
        if (fabsf(bpm - prev_bpm) > 0.5f) {
            UpdateClock();
            DEBUG_SEQ_NORMAL("TEMPO CHANGED: %.1f -> %.1f BPM", prev_bpm, bpm);
        }
        
        // Knob 1: Swing (0.0 - 0.5)
        float prev_swing = swing_amount;
        swing_amount = hw.knob[1].Value() * 0.5f;
        if (fabsf(swing_amount - prev_swing) > 0.01f) {
            Debug_ReportParameterChange("Swing", prev_swing, swing_amount, "Knob1");
        }
        
        // Knob 2: FM Index (oscillator control)
        fm_index = hw.knob[2].Value() * 10.0f;
        Debug_ReportParameterChange("FM Index", prev_fm_index, fm_index, "Knob2");
        prev_fm_index = fm_index;
        
        // Knob 3: Filter Cutoff
        cutoff = 100.0f + hw.knob[3].Value() * 9900.0f;
        Debug_ReportParameterChange("Filter Cutoff", prev_cutoff, cutoff, "Knob3");
        prev_cutoff = cutoff;
        
        // Knob 4: Resonance
        resonance = hw.knob[4].Value() * 0.95f;
        Debug_ReportParameterChange("Filter Resonance", prev_resonance, resonance, "Knob4");
        prev_resonance = resonance;
        
        // Knob 5: Formant
        formant = 200.0f + hw.knob[5].Value() * 4800.0f;
        Debug_ReportParameterChange("Formant", prev_formant, formant, "Knob5");
        prev_formant = formant;
        
        // Knob 6: Drive
        drive = hw.knob[6].Value();
        Debug_ReportParameterChange("Overdrive", prev_drive, drive, "Knob6");
        prev_drive = drive;
        
        // Knob 7: Reverb
        reverb_feedback = hw.knob[7].Value() * 0.8f;
        Debug_ReportParameterChange("Reverb Feedback", prev_reverb_feedback, reverb_feedback, "Knob7");
        prev_reverb_feedback = reverb_feedback;
        
        // Default oscillator settings
        fm_ratio = 1.0f;
        grain_shape = 0.5f;
        
    } else { // MODE_EDIT
        // In edit mode, knobs control selected step parameters
        if(selected_step >= 0 && selected_step < NUM_STEPS) {
            // Knob 0: Step frequency (transpose ±2 octaves)
            float base_freq = KEY_FREQUENCIES[selected_step];
            float transpose = (hw.knob[0].Value() - 0.5f) * 4.0f;  // ±2 octaves
            float new_freq = base_freq * powf(2.0f, transpose);
            Debug_ReportParameterChange("Step Freq", sequence[selected_step].frequency, new_freq, "Knob0 Edit");
            sequence[selected_step].frequency = new_freq;
            
            // Knob 1: Velocity
            float new_velocity = hw.knob[1].Value();
            Debug_ReportParameterChange("Step Velocity", sequence[selected_step].velocity, new_velocity, "Knob1 Edit");
            sequence[selected_step].velocity = new_velocity;
            
            // Knob 2: Gate length
            float new_gate = 0.1f + hw.knob[2].Value() * 0.9f;
            Debug_ReportParameterChange("Gate Length", sequence[selected_step].gate_length, new_gate, "Knob2 Edit");
            sequence[selected_step].gate_length = new_gate;
            
            // Knob 3: Toggle step active (threshold at 0.5)
            bool new_active = (hw.knob[3].Value() > 0.5f);
            if (sequence[selected_step].active != new_active) {
                DEBUG_UI_NORMAL("STEP %d ACTIVITY: %s -> %s", selected_step,
                               sequence[selected_step].active ? "ACTIVE" : "INACTIVE",
                               new_active ? "ACTIVE" : "INACTIVE");
                sequence[selected_step].active = new_active;
            }
        }
        
        // Keep playback parameters at defaults during edit
        fm_ratio = 1.0f;
        fm_index = 2.0f;
        formant = 1000.0f;
        grain_shape = 0.5f;
        cutoff = 5000.0f;
        resonance = 0.3f;
        drive = 0.0f;
        reverb_feedback = 0.3f;
    }
    
    // Read all 16 key states
    for(int i = 0; i < 16; i++) {
        key_state[i] = hw.KeyboardState(i);
    }
    
    // Handle key presses (bottom row controls)
    // B1 (key 8): START/STOP
    if(key_state[8] && !key_prev[8]) {
        sequencer_running = !sequencer_running;
        if(sequencer_running) {
            current_step = 0;
            DEBUG_SEQ_NORMAL("SEQUENCER STARTED");
        } else {
            DEBUG_SEQ_NORMAL("SEQUENCER STOPPED");
        }
    }
    
    // B2 (key 9): RESET
    if(key_state[9] && !key_prev[9]) {
        current_step = 0;
        DEBUG_SEQ_NORMAL("SEQUENCER RESET to step 0");
    }
    
    // B3 (key 10): RECORD MODE TOGGLE
    if(key_state[10] && !key_prev[10]) {
        ControlMode prev_mode = current_mode;
        current_mode = (current_mode == MODE_RECORD) ? MODE_PLAY : MODE_RECORD;
        DEBUG_UI_NORMAL("MODE CHANGE: %s -> %s", 
                       prev_mode == MODE_PLAY ? "PLAY" : "RECORD",
                       current_mode == MODE_PLAY ? "PLAY" : "RECORD");
    }
    
    // B4 (key 11): CLEAR SELECTED STEP
    if(key_state[11] && !key_prev[11]) {
        if(current_mode == MODE_EDIT && selected_step >= 0 && selected_step < NUM_STEPS) {
            sequence[selected_step].active = false;
            DEBUG_UI_NORMAL("STEP %d CLEARED (set inactive)", selected_step);
        }
    }
    
    // Handle key presses (top row - step selection/programming)
    if(current_mode == MODE_RECORD) {
        // In record mode, pressing top row keys sets that step's frequency
        for(int i = 0; i < 8; i++) {
            if(key_state[i] && !key_prev[i]) {
                sequence[i].frequency = KEY_FREQUENCIES[i];
                sequence[i].active = true;
                sequence[i].velocity = 0.8f;
                DEBUG_SEQ_NORMAL("STEP %d RECORDED: Freq=%.2f Hz", i, KEY_FREQUENCIES[i]);
            }
        }
    } else if(current_mode == MODE_EDIT) {
        // In edit mode, select step for parameter editing
        for(int i = 0; i < 8; i++) {
            if(key_state[i] && !key_prev[i]) {
                selected_step = i;
                DEBUG_UI_NORMAL("EDIT MODE: Selected step %d for editing", selected_step);
            }
        }
    }
    
    // Update key_prev
    for(int i = 0; i < 16; i++) {
        key_prev[i] = key_state[i];
    }
    
    // Set FM parameters with debug
    fm1.SetRatio(fm_ratio);
    fm1.SetIndex(fm_index);
    fm2.SetRatio(fm_ratio);
    fm2.SetIndex(fm_index);
    
    DEBUG_FM_VERBOSE("FM Parameters - Ratio: %.2f, Index: %.2f", fm_ratio, fm_index);
    
    // Set granular parameters with debug
    grainlet1.SetFormantFreq(formant);
    grainlet1.SetShape(grain_shape);
    grainlet2.SetFormantFreq(formant);
    grainlet2.SetShape(grain_shape);
    
    DEBUG_GRANULAR_VERBOSE("Granular Parameters - Formant: %.0f Hz, Shape: %.2f", 
                          formant, grain_shape);
    
    // Set filter parameters with debug
    filter1.SetFreq(cutoff);
    filter1.SetRes(resonance);
    filter2.SetFreq(cutoff);
    filter2.SetRes(resonance);
    
    DEBUG_FM_VERBOSE("Filter Parameters - Cutoff: %.0f Hz, Resonance: %.2f", 
                    cutoff, resonance);
    
    // Set overdrive
    overdrive.SetDrive(drive);
    
    // Set reverb
    reverb.SetFeedback(reverb_feedback);
    
    DEBUG_FM_VERBOSE("Effects - Drive: %.2f, Reverb: %.2f", drive, reverb_feedback);
    
    // Process audio sample by sample
    for (size_t i = 0; i < size; i++) {
        // Sequencer clock logic
        if(sequencer_running && clock.Process()) {
            // Advance to next step
            int prev_step = current_step;
            current_step = (current_step + 1) % NUM_STEPS;
            TriggerStep(current_step);
            DEBUG_SEQ_VERBOSE("STEP ADVANCE: %d -> %d", prev_step, current_step);
        }
        
        // Gate timer - turn off gate after gate_length fraction of step
        if(gate_active && gate_timer.Process()) {
            Step& step = sequence[current_step];
            // Simple gate counter (approximate)
            static int gate_counter = 0;
            gate_counter++;
            int gate_threshold = static_cast<int>(10.0f * step.gate_length);
            if(gate_counter >= gate_threshold) {
                gate_active = false;
                gate_counter = 0;
                DEBUG_ENV_VERBOSE("GATE CLOSED: Step %d, Duration: %.2f", current_step, step.gate_length);
            }
        }
        
        // Process envelopes (AdEnv uses triggers, not gates)
        bool trigger = step_triggered;
        step_triggered = false;  // One-shot trigger
        
        // Trigger envelopes if step was triggered this cycle
        if(trigger) {
            env1.Trigger();
            env2.Trigger();
            DEBUG_ENV_VERBOSE("ENV TRIGGERED: Both voices");
        }
        
        float env1_out = env1.Process();
        float env2_out = env2.Process();
        
        // Apply velocity scaling from current step
        float velocity = sequence[current_step].velocity;
        env1_out *= velocity;
        env2_out *= velocity;
        
        DEBUG_ENV_VERBOSE("ENV Levels: V1=%.3f, V2=%.3f (vel=%.2f)", 
                         env1_out, env2_out, velocity);
        
        // Voice 1 signal chain
        float fm1_out = fm1.Process();
        float grain1_out = grainlet1.Process();
        float mixed1 = fm1_out * 0.5f + grain1_out * 0.5f;
        float filt1_out = filter1.Process(mixed1);
        float vca1_out = filt1_out * env1_out;
        
        // Voice 2 signal chain
        float fm2_out = fm2.Process();
        float grain2_out = grainlet2.Process();
        float mixed2 = fm2_out * 0.5f + grain2_out * 0.5f;
        float filt2_out = filter2.Process(mixed2);
        float vca2_out = filt2_out * env2_out;
        
        // Mix voices
        float mixed = (vca1_out + vca2_out) * 0.5f;
        
        // Apply overdrive
        float driven = overdrive.Process(mixed);
        
        // Apply reverb (stereo)
        float reverb_l, reverb_r;
        reverb.Process(driven, driven, &reverb_l, &reverb_r);
        
        // Output to stereo
        out[0][i] = reverb_l;
        out[1][i] = reverb_r;
        
        // Debug audio levels
        if (i == 0) { // Only debug first sample of block to avoid spam
            Debug_ReportAudioLevels(reverb_l, reverb_r);
        }
    }
    
    // Report performance periodically
    Debug_ReportPerformance();
}

// ========================================
// MAIN FUNCTION WITH DEBUG INITIALIZATION
// ========================================

int main(void) {
    // Initialize debug system
    g_debug_enabled = true;
    g_debug_level = DEBUG_NORMAL;
    g_debug_subsystems = DEBUG_ALL;
    
    DEBUG_SEQ_MINIMAL("=== DAISY FIELD FM-GRANULAR SEQUENCER STARTING ===");
    DEBUG_SEQ_MINIMAL("Debug Level: NORMAL, Subsystems: ALL");
    
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sample_rate = hw.AudioSampleRate();
    
    DEBUG_AUDIO_NORMAL("Hardware initialized - Sample Rate: %.0f Hz, Block Size: 48", sample_rate);
    
    // Initialize sequencer
    InitializeSequence();
    
    // Initialize clock
    clock.Init(sample_rate / 48.0f, bpm / 60.0f * 4.0f);  // Block rate, 16th notes
    gate_timer.Init(sample_rate / 48.0f, (bpm / 60.0f * 4.0f) * 10.0f);
    
    DEBUG_SEQ_NORMAL("Clock initialized - BPM: %.1f", bpm);
    
    // Initialize DSP modules - Voice 1
    fm1.Init(sample_rate);
    fm1.SetFrequency(130.81f);
    
    grainlet1.Init(sample_rate);
    grainlet1.SetFreq(130.81f);
    grainlet1.SetBleed(0.3f);
    
    filter1.Init(sample_rate);
    
    env1.Init(sample_rate);
    env1.SetTime(ADENV_SEG_ATTACK, 0.005f);   // 5ms attack
    env1.SetTime(ADENV_SEG_DECAY, 0.2f);      // 200ms decay
    env1.SetCurve(-20.0f);                     // Exponential decay
    
    DEBUG_FM_NORMAL("Voice 1 initialized - FM: C3, Env: 5ms/200ms");
    
    // Initialize DSP modules - Voice 2
    fm2.Init(sample_rate);
    fm2.SetFrequency(130.81f);
    
    grainlet2.Init(sample_rate);
    grainlet2.SetFreq(130.81f);
    grainlet2.SetBleed(0.3f);
    
    filter2.Init(sample_rate);
    
    env2.Init(sample_rate);
    env2.SetTime(ADENV_SEG_ATTACK, 0.005f);
    env2.SetTime(ADENV_SEG_DECAY, 0.2f);
    env2.SetCurve(-20.0f);
    
    DEBUG_FM_NORMAL("Voice 2 initialized - FM: C3, Env: 5ms/200ms");
    
    // Initialize effects
    overdrive.Init();
    
    reverb.Init(sample_rate);
    reverb.SetFeedback(0.6f);
    reverb.SetLpFreq(8000.0f);
    
    DEBUG_AUDIO_NORMAL("Effects initialized - Overdrive, Reverb (fb=0.6, lp=8kHz)");
    
    // Start audio and ADC
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    DEBUG_SEQ_MINIMAL("Audio system started - Ready for operation");
    
    // Main loop
    while(1) {
        // Update LEDs to show sequencer state
        for(int i = 0; i < 8; i++) {
            float brightness = 0.0f;
            
            if(current_mode == MODE_PLAY) {
                // Show current step (bright), active steps (dim)
                if(i == current_step && sequencer_running) {
                    brightness = 1.0f;
                } else if(sequence[i].active) {
                    brightness = 0.2f;
                }
            } else if(current_mode == MODE_RECORD) {
                // Show active steps, bright if being pressed
                if(hw.KeyboardState(i)) {
                    brightness = 1.0f;
                } else if(sequence[i].active) {
                    brightness = 0.3f;
                }
            } else if(current_mode == MODE_EDIT) {
                // Show selected step (bright), others (dim)
                if(i == selected_step) {
                    brightness = 1.0f;
                } else if(sequence[i].active) {
                    brightness = 0.2f;
                }
            }
            
            hw.led_driver.SetLed(i, brightness);
        }
        
        // Bottom row LEDs (control indicators)
        hw.led_driver.SetLed(8, sequencer_running ? 1.0f : 0.1f);  // START/STOP
        hw.led_driver.SetLed(9, 0.1f);                              // RESET
        hw.led_driver.SetLed(10, (current_mode == MODE_RECORD) ? 1.0f : 0.1f);  // RECORD MODE
        hw.led_driver.SetLed(11, 0.1f);                             // CLEAR STEP
        
        hw.led_driver.SwapBuffersAndTransmit();
        
        System::Delay(10);
    }
}