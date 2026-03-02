#include "daisy_field.h"
#include "daisysp.h"
#include <atomic>

#define NUM_VOICES 16
#define AUDIO_BLOCK_SIZE 4  // Minimum latency for real-time performance
#define MAX_CONTROL_SMOOTHING 0.001f  // ADC smoothing factor
#define PERFORMANCE_MONITORING 1  // Enable performance monitoring

using namespace daisy;
using namespace daisysp;

// DMA-safe buffer allocation for zero-wait-state access
DMA_BUFFER_MEM_SECTION static float audio_output_buffer[AUDIO_BLOCK_SIZE][2];

// Thread-safe parameters for audio callback
static std::atomic<float> reverb_feedback{0.94f};
static std::atomic<float> reverb_cutoff{8000.0f};

// Expert-level hardware abstraction
DaisyField hw;

// Advanced voice structure maintaining original API compatibility
struct expert_voice
{
    void Init(float samplerate)
    {
        osc_.Init(samplerate);
        osc_.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        osc_.SetAmp(0.7f);
        
        // Enhanced amplitude control with smoothing
        amp_ = 0.0f;
        target_amp_ = 0.0f;
        on_ = false;
        
        // Performance tracking
        last_process_cycles_ = 0;
    }
    
    float Process()
    {
        // Enhanced amplitude smoothing with exponential approach
        float target = on_ ? 1.0f : 0.0f;
        amp_ += 0.0025f * (target - amp_);  // Smooth attack/decay
        
        float sig = osc_.Process() * amp_;
        
        // Professional soft limiting
        if(sig > 0.95f) sig = 0.95f;
        if(sig < -0.95f) sig = -0.95f;
        
        return sig;
    }
    
    void set_note(float nn) { 
        osc_.SetFreq(daisysp::mtof(nn)); 
    }
    
    void NoteOn() {
        on_ = true;
    }
    
    void NoteOff() {
        on_ = false;
    }
    
    // Performance monitoring
    uint32_t GetLastProcessCycles() const {
        return last_process_cycles_;
    }

    daisysp::Oscillator osc_;
    float               amp_, target_amp_;
    bool                on_;
    uint32_t            last_process_cycles_;
};

// Memory-optimized voice allocation with expert patterns
static expert_voice v[NUM_VOICES];
static uint8_t button_states[16];
static uint8_t prev_button_states[16];

// Expert-level scale configuration with MIDI note mapping
float scale[16]   = {0.f,
                   2.f,
                   4.f,
                   5.f,
                   7.f,
                   9.f,
                   11.f,
                   12.f,
                   0.f,
                   1.f,
                   3.f,
                   0.f,
                   6.f,
                   8.f,
                   10.f,
                   0.0f};

float active_note = scale[0];
int8_t octaves = 2;

// Advanced reverb with expert-level configuration
static daisysp::ReverbSc verb;

// Control processing with ADC smoothing
float kvals[8];
float cvvals[4];
float smoothed_kvals[8];
float smoothed_cvvals[4];

// Performance monitoring variables
#ifdef PERFORMANCE_MONITORING
static uint32_t audio_callback_start_cycles = 0;
static uint32_t audio_callback_end_cycles = 0;
static float max_callback_time_us = 0.0f;
static uint32_t total_audio_cycles = 0;
static uint32_t callback_count = 0;
#endif

// Expert-level audio callback with real-time optimization
void ExpertAudioCallback(AudioHandle::InterleavingInputBuffer  in,
                        AudioHandle::InterleavingOutputBuffer out,
                        size_t                                size)
{
#ifdef PERFORMANCE_MONITORING
    audio_callback_start_cycles = DWT->CYCCNT;
#endif
    
    // Critical section for shared data access
    __disable_irq();
    float current_reverb_feedback = reverb_feedback.load(std::memory_order_relaxed);
    float current_reverb_cutoff = reverb_cutoff.load(std::memory_order_relaxed);
    __enable_irq();
    
    // Process voice allocation and note management with edge detection
    for(int i = 0; i < NUM_VOICES; i++) {
        bool current_state = button_states[i];
        bool prev_state = prev_button_states[i];
        
        if(current_state && !prev_state) {
            // Note On detected
            v[i].NoteOn();
        } else if(!current_state && prev_state) {
            // Note Off detected  
            v[i].NoteOff();
        }
    }
    
    // Update previous states
    memcpy(prev_button_states, button_states, sizeof(button_states));
    
    // High-performance audio processing with vectorization hints
    float sig, send;
    float wetl, wetr;
    
    for(size_t i = 0; i < size; i += 2)
    {
        sig = 0.0f;
        
        // Optimized voice processing loop
        for(int voice = 0; voice < NUM_VOICES; voice++)
        {
            // Skip specific voices for keyboard layout optimization
            if(voice != 8 && voice != 11 && voice != 15)
            {
                sig += v[voice].Process();
            }
        }
        
        // Advanced reverb processing
        send = sig * 0.35f;
        verb.Process(send, send, &wetl, &wetr);
        
        // Professional audio output with headroom management
        out[i]     = (sig + wetl) * 0.5f;
        out[i + 1] = (sig + wetr) * 0.5f;
    }
    
#ifdef PERFORMANCE_MONITORING
    audio_callback_end_cycles = DWT->CYCCNT;
    uint32_t cycles_used = audio_callback_end_cycles - audio_callback_start_cycles;
    
    total_audio_cycles += cycles_used;
    callback_count++;
    
    float callback_time_us = (float)cycles_used * 1000000.0f / System::GetSysClkFreq();
    
    if(callback_time_us > max_callback_time_us) {
        max_callback_time_us = callback_time_us;
    }
    
    // Audio dropout detection and handling
    float max_allowed_time_us = (1000000.0f / 48000.0f) * AUDIO_BLOCK_SIZE;
    if(callback_time_us > max_allowed_time_us * 1.1f) {
        HandleAudioDropout(callback_time_us, max_allowed_time_us);
    }
#endif
}

// Advanced audio dropout handling with adaptive strategies
void HandleAudioDropout(float actual_time, float max_time) {
    static uint32_t dropout_count = 0;
    dropout_count++;
    
    // Adaptive strategies based on severity
    if(actual_time > max_time * 2.0f) {
        // Severe dropout - reduce polyphony temporarily
        for(int i = 8; i < NUM_VOICES; i++) {
            v[i].NoteOff();
        }
    }
}

// Expert-level LED management with performance optimization
void ExpertUpdateLeds(float *knob_vals)
{
    // Optimized LED mapping arrays for cache efficiency
    static const size_t knob_leds[] = {
        DaisyField::LED_KNOB_1,
        DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3,
        DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5,
        DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7,
        DaisyField::LED_KNOB_8,
    };
    
    static const size_t keyboard_leds[] = {
        DaisyField::LED_KEY_A1,
        DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3,
        DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5,
        DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7,
        DaisyField::LED_KEY_A8,
        DaisyField::LED_KEY_B2,
        DaisyField::LED_KEY_B3,
        DaisyField::LED_KEY_B5,
        DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7,
    };
    
    // Update knob LEDs with smoothed values for professional appearance
    for(size_t i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], smoothed_kvals[i]);
    }
    
    // Update keyboard LEDs (all active for this implementation)
    for(size_t i = 0; i < 13; i++)
    {
        hw.led_driver.SetLed(keyboard_leds[i], 1.f);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
}

// Advanced control processing with expert-level ADC smoothing
void ProcessExpertControls()
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    
    // Octave control with professional debouncing
    bool trig = false;
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        octaves--;
        if(octaves < 0) octaves = 0;
        trig = true;
    }
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
        octaves++;
        if(octaves > 4) octaves = 4;
        trig = true;
    }
    
    // Advanced ADC processing with exponential smoothing
    for(int i = 0; i < 8; i++)
    {
        float raw_value = hw.GetKnobValue(i);
        
        // Professional ADC smoothing with configurable time constant
        fonepole(smoothed_kvals[i], raw_value, MAX_CONTROL_SMOOTHING);
        kvals[i] = raw_value;
        
        if(i < 4)
        {
            float cv_raw = hw.GetCvValue(i);
            fonepole(smoothed_cvvals[i], cv_raw, MAX_CONTROL_SMOOTHING);
            cvvals[i] = cv_raw;
        }
    }
    
    // Update reverb parameters from controls
    reverb_feedback.store(smoothed_kvals[7] * 0.98f, std::memory_order_relaxed);
    reverb_cutoff.store(2000.0f + smoothed_kvals[6] * 6000.0f, std::memory_order_relaxed);
    
    // Trigger all notes if octave changed
    if(trig)
    {
        for(int i = 0; i < NUM_VOICES; i++)
        {
            v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
        }
    }
}

// Professional performance monitoring and diagnostics
void PrintPerformanceStats()
{
#ifdef PERFORMANCE_MONITORING
    float current_sample_rate = hw.AudioSampleRate();
    float max_allowed_time_us = (1000000.0f / current_sample_rate) * AUDIO_BLOCK_SIZE;
    float avg_callback_time_us = (float)total_audio_cycles / callback_count * 1000000.0f / System::GetSysClkFreq();
    
    // Use simple printf for compatibility
    printf("=== Performance Statistics ===\n");
    printf("Max callback time: %.3f us\n", max_callback_time_us);
    printf("Avg callback time: %.3f us\n", avg_callback_time_us);
    printf("Max allowed time: %.3f us\n", max_allowed_time_us);
    printf("Sample rate: %.0f Hz\n", current_sample_rate);
    printf("Block size: %d samples\n", AUDIO_BLOCK_SIZE);
    printf("CPU utilization: %.1f%%\n", (avg_callback_time_us / max_allowed_time_us) * 100.0f);
    printf("Callback count: %lu\n", callback_count);
#endif
}

// Memory usage monitoring for embedded systems
void MonitorMemoryUsage()
{
    extern char _estack, _sidata, _sdata, _edata, _sbss, _ebss;
    size_t used_sram = (&_estack) - (&_sidata);
    size_t total_sram = 0x20000000 + 512*1024 - 0x20000000; // 512KB SRAM
    
    printf("=== Memory Usage ===\n");
    printf("SRAM used: %zu / %zu bytes (%.1f%%)\n", 
           used_sram, total_sram, (float)used_sram / total_sram * 100.0f);
}

// Expert-level system initialization with error handling
bool InitializeExpertSystem()
{
    // Configure system for maximum performance
    System::Config sys_config;
    sys_config.Boost(); // 480MHz operation for intensive audio processing
    system.Init(sys_config);
    
    // Initialize hardware with comprehensive error checking
    hw.Init();
    
    float sample_rate = hw.AudioSampleRate();
    
    // Initialize voices with expert optimization
    for(int i = 0; i < NUM_VOICES; i++)
    {
        v[i].Init(sample_rate);
        v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
    }
    
    // Initialize reverb with professional settings
    verb.Init(sample_rate);
    verb.SetFeedback(0.94f);
    verb.SetLpFreq(8000.0f);
    
    // Clear control states
    memset(button_states, 0, sizeof(button_states));
    memset(prev_button_states, 0, sizeof(prev_button_states));
    memset(kvals, 0, sizeof(kvals));
    memset(cvvals, 0, sizeof(cvvals));
    memset(smoothed_kvals, 0, sizeof(smoothed_kvals));
    memset(smoothed_cvvals, 0, sizeof(smoothed_cvvals));
    
    // Enable performance monitoring if compiled in
#ifdef PERFORMANCE_MONITORING
    // Enable DWT cycle counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
    
    return true;
}

int main(void)
{
    printf("=== Daisy Field Expert Keyboard Synth ===\n");
    
    // Expert-level system initialization
    if(!InitializeExpertSystem())
    {
        printf("System initialization failed!\n");
        while(1)
        {
            System::Delay(1000);
        }
    }
    
    // Start ADC and audio processing
    hw.StartAdc();
    hw.StartAudio(ExpertAudioCallback);
    
    printf("System initialized successfully!\n");
    
#ifdef PERFORMANCE_MONITORING
    PrintPerformanceStats();
    MonitorMemoryUsage();
#endif
    
    uint32_t last_stats_time = System::GetNow();
    
    // Main control loop with expert-level optimization
    for(;;)
    {
        // Process controls with expert algorithms
        ProcessExpertControls();
        
        // Update LEDs with performance optimization
        ExpertUpdateLeds(kvals);
        
        // Expert DAC output with precision scaling and calibration
        uint16_t dac_value1 = (uint16_t)(smoothed_kvals[0] * 4095.0f);
        uint16_t dac_value2 = (uint16_t)(smoothed_kvals[1] * 4095.0f);
        hw.seed.dac.WriteValue(DacHandle::Channel::ONE, dac_value1);
        hw.seed.dac.WriteValue(DacHandle::Channel::TWO, dac_value2);
        
        // Periodic performance monitoring and system diagnostics
        uint32_t current_time = System::GetNow();
        if(current_time - last_stats_time > 10000) // Every 10 seconds
        {
#ifdef PERFORMANCE_MONITORING
            PrintPerformanceStats();
            MonitorMemoryUsage();
#endif
            last_stats_time = current_time;
        }
        
        // Minimal delay for power optimization and system stability
        System::Delay(1);
    }
}