// ============================================================
// PodTonestack_NlFilt.cpp — Daisy Pod + ToneStack + NlFilt Filter
// Prototype for Pod_OledEncoder_test
//
// DSP Chain: ToneStack → NlFilt (serial)
// Control Modes:
//   Default (no SW held)  → ToneStack: Bass/Knob1, Mid/Encoder, Treble/Knob2
//   SW1 held (LFO Mode)  → LFO Rate/Knob1, Depth/Knob2, Wave/Encoder
//   SW2 held (NL Mode)   → Attack/Knob1, Release/Knob2, Coef d/Encoder
//
// Build:  make
// Flash:  make program
// ============================================================

#include "daisy_pod.h"
#include "daisysp.h"

#include "OledUI.h"
#include "PodControls.h"

// DAFX_2_Daisy_lib headers
#include "effects/tonestack.h"

using namespace daisy;
using namespace daisysp;

// ---- Global objects ----------------------------------------
static DaisyPod    pod;
static OledUI      ui;
static PodControls ctl;

// ---- DSP Modules -------------------------------------------
static ToneStack   toneStack;
static NlFilt      nlFilt;
static Oscillator  lfo;

// ---- State variables ---------------------------------------
static float sample_rate = 0.f;

// Mode tracking
enum ControlMode { MODE_TONE, MODE_LFO, MODE_NL };
static ControlMode current_mode = MODE_TONE;

// Bypass states
static bool tone_bypass = false;
static bool nlfilt_bypass = false;

// NlFilt coefficients
static float nlfilt_a = 0.f;
static float nlfilt_b = 0.f;
static float nlfilt_d = 0.5f;
static float nlfilt_C = 0.f;
static float nlfilt_L = 10.f;

// ---- Initialize DSP ----------------------------------------
static void InitDSP() {
    sample_rate = pod.AudioSampleRate();
    
    // Init ToneStack
    toneStack.Init(sample_rate);
    toneStack.SetBass(0.f);
    toneStack.SetMiddle(0.f);
    toneStack.SetTreble(0.f);
    
    // Init NlFilt (LGPL module)
    nlFilt.Init();
    nlFilt.SetCoefficients(nlfilt_a, nlfilt_b, nlfilt_d, nlfilt_C, (int)nlfilt_L);
    
    // Init LFO
    lfo.Init(sample_rate);
    lfo.SetFreq(1.f);
    lfo.SetAmp(0.5f);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
}

// ---- Update parameters based on mode -----------------------
static void UpdateParams() {
    // Get raw param values (0-1)
    float p0 = ctl.GetParam(0);
    float p1 = ctl.GetParam(1);
    float p2 = ctl.GetParam(2);
    
    switch(current_mode) {
        case MODE_TONE: {
            // Map 0-1 to -1 to +1 for ToneStack
            float bass = p0 * 2.f - 1.f;
            float mid = p2 * 2.f - 1.f;
            float treble = p1 * 2.f - 1.f;
            toneStack.SetBass(bass);
            toneStack.SetMiddle(mid);
            toneStack.SetTreble(treble);
            break;
        }
        case MODE_LFO: {
            // LFO Rate: log mapping 0.1-10 Hz from p0
            float rate = 0.1f * powf(10.f, p0);
            lfo.SetFreq(rate);
            // LFO Depth: p1
            lfo.SetAmp(p1);
            // Wave: p2 * 3 -> 0-3
            int wave = (int)(p2 * 3.99f);
            lfo.SetWaveform((uint8_t)wave);
            break;
        }
        case MODE_NL: {
            // Attack: log 0.01-2s from p0
            float attack = 0.01f * powf(200.f, p0);
            // Release: log 0.1-5s from p1
            float release = 0.1f * powf(50.f, p1);
            // Coef d: 0-3 from p2
            nlfilt_d = p2 * 3.f;
            nlFilt.SetCoefficients(nlfilt_a, nlfilt_b, nlfilt_d, nlfilt_C, (int)nlfilt_L);
            // Use attack/release to suppress warnings (future: envelope follower)
            (void)attack;
            (void)release;
            break;
        }
    }
}

// ---- Process LFO modulation --------------------------------
static void ProcessLFO() {
    if(current_mode != MODE_LFO) return;
    
    float lfo_out = lfo.Process();
    // Modulate NlFilt coef d
    float base_d = 0.5f;
    float modulated_d = base_d + lfo_out * ctl.GetParam(1) * 2.f;  // depth
    if(modulated_d < 0.f) modulated_d = 0.f;
    if(modulated_d > 3.f) modulated_d = 3.f;
    nlFilt.SetCoefficients(nlfilt_a, nlfilt_b, modulated_d, nlfilt_C, (int)nlfilt_L);
}

// ---- Audio callback -----------------------------------------
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Process LFO modulation
    ProcessLFO();
    
    for(size_t i = 0; i < size; i++) {
        float sig = in[0][i];  // Mono input
        
        // ToneStack
        if(!tone_bypass) {
            sig = toneStack.Process(sig);
        }
        
        // NlFilt
        if(!nlfilt_bypass) {
            nlFilt.ProcessBlock(&sig, &sig, 1);
        }
        
        // Output
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

// ---- Main --------------------------------------------------
int main(void) {
    // Init hardware
    pod.Init();
    pod.SetAudioBlockSize(4);
    
    // Init UI (using default Seed pins - may need adjustment for Pod)
    ui.Init();
    
    // Init controls
    ctl.Init(pod, ui);
    ctl.SetParamLabel(0, "Bass/Rate");
    ctl.SetParamLabel(1, "Treble/Depth");
    ctl.SetParamLabel(2, "Mid/Coef");
    
    // Init DSP
    InitDSP();
    
    // Start audio
    pod.StartAdc();
    pod.StartAudio(AudioCallback);
    
    // Main loop
    while(true) {
        // Poll controls (includes encoder, knobs, buttons)
        ctl.Poll();
        
        // Determine mode based on button hold state
        // Note: Using button1 as SW1, button2 as SW2
        // In Tone Mode (default): buttons are bypass toggles
        // In LFO Mode (SW1 held): SW1 = button1 held
        // In NL Mode (SW2 held): SW2 = button2 held
        
        bool sw1_held = pod.button1.Pressed();
        bool sw2_held = pod.button2.Pressed();
        
        if(sw2_held) {
            current_mode = MODE_NL;
        } else if(sw1_held) {
            current_mode = MODE_LFO;
        } else {
            current_mode = MODE_TONE;
        }
        
        // Handle bypass toggles (only when not in mode-hold)
        if(!sw1_held && pod.button1.RisingEdge()) {
            tone_bypass = !tone_bypass;
        }
        if(!sw2_held && pod.button2.RisingEdge()) {
            nlfilt_bypass = !nlfilt_bypass;
        }
        
        // Update DSP parameters based on current mode
        UpdateParams();
        
        // Update LEDs based on bypass state
        if(tone_bypass) pod.led1.Set(0.f, 0.f, 0.f);
        else pod.led1.Set(0.f, 1.f, 0.f);
        
        if(nlfilt_bypass) pod.led2.Set(0.f, 0.f, 0.f);
        else pod.led2.Set(0.f, 1.f, 0.f);
    }
}