/**
 * Dual Oscillator Subtractive Synthesizer
 * Platform: Daisy Field
 * 
 * Features:
 * - 2 oscillators with detune
 * - SVF lowpass filter with resonance
 * - Dual ADSR envelopes (amplitude + filter)
 * - Piano-style keyboard layout
 * - Octave switching via switches

Control Mapping
Control	Parameter	        Range
Knob 0	OSC Waveform	    SAW ↔ SQUARE
Knob 1	OSC2 Detune	        -12 to +12 semitones
Knob 2	OSC Mix	            OSC1 ↔ OSC2
Knob 3	Filter Cutoff	    50-10,000 Hz
Knob 4	Filter Resonance	0-95%
Knob 5	Filter Env Amount	0-100%
Knob 6	Attack	            5ms-2s
Knob 7	Release	            10ms-3s
SW1	    Octave Down	        0-4
SW2	    Octave Up	        0-4
Keyboard	Piano Layout	    Notes + accidentals

 */

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// DSP Objects
Oscillator osc1, osc2;
Svf        filter;
Adsr       ampEnv, filtEnv;

// Keyboard state
uint8_t buttons[16];

// Piano-style layout: Major scale on bottom, accidentals on top
// Keys 8, 11, 15 are unused (black key gaps)
float scale[16] = {
    0.f, 2.f, 4.f, 5.f, 7.f, 9.f, 11.f, 12.f,   // C D E F G A B C (bottom row)
    0.f, 1.f, 3.f, 0.f, 6.f, 8.f, 10.f, 0.0f    // C# D# - F# G# A# - (top row)
};

float    active_note = scale[0];
int8_t   octave      = 2;      // Base octave (C2 = MIDI 36)
bool     gate        = false;
uint8_t  active_key  = 255;    // No key active

// Parameter values
float osc1_wave   = 0.f;
float osc2_detune = 0.f;
float osc_mix     = 0.5f;
float filt_cutoff = 2000.f;
float filt_res    = 0.3f;
float filt_env_amt = 0.5f;
float attack_time = 0.01f;
float release_time = 0.2f;

// Previous knob values for change detection
float prev_knobs[8] = {0.f};
const float KNOB_THRESHOLD = 0.01f;

// Active parameter display
int8_t   active_param = -1;     // -1 = no param active, 0-7 = knob index
uint32_t param_display_time = 0;
const uint32_t PARAM_TIMEOUT_MS = 1000;

// Parameter names
const char* param_names[8] = {
    "WAVEFORM", "OSC2 DETUNE", "OSC MIX", "FILTER CUT",
    "FILTER RES", "FILT ENV", "ATTACK", "RELEASE"
};

// Convert semitone to frequency
float SemiToFreq(float semi)
{
    return 440.f * powf(2.f, (semi - 69.f) / 12.f);
}

void UpdateOled()
{
    hw.display.Fill(false);
    char buf[24];
    
    // Check if we should show active parameter
    if(active_param >= 0 && System::GetNow() - param_display_time < PARAM_TIMEOUT_MS)
    {
        // Show parameter name (large)
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(param_names[active_param], Font_7x10, true);
        
        // Show parameter value (large)
        hw.display.SetCursor(0, 20);
        switch(active_param)
        {
            case 0:  // Waveform
                sprintf(buf, "%s", osc1_wave < 0.5f ? "SAW" : "SQUARE");
                break;
            case 1:  // OSC2 Detune
                sprintf(buf, "%+.1f semi", osc2_detune);
                break;
            case 2:  // OSC Mix
                sprintf(buf, "%.0f%% OSC2", osc_mix * 100.f);
                break;
            case 3:  // Filter Cutoff
                sprintf(buf, "%d Hz", (int)filt_cutoff);
                break;
            case 4:  // Filter Resonance
                sprintf(buf, "%.0f%%", (filt_res / 0.95f) * 100.f);
                break;
            case 5:  // Filter Env Amount
                sprintf(buf, "%.0f%%", filt_env_amt * 100.f);
                break;
            case 6:  // Attack
                sprintf(buf, "%.0f ms", attack_time * 1000.f);
                break;
            case 7:  // Release
                sprintf(buf, "%.0f ms", release_time * 1000.f);
                break;
        }
        hw.display.WriteString(buf, Font_11x18, true);
        
        // Progress bar
        float knob_val = hw.knob[active_param].Process();
        int bar_width = (int)(knob_val * 120.f);
        hw.display.DrawRect(0, 50, 127, 58, true, false);    // Outline
        hw.display.DrawRect(1, 51, bar_width, 57, true, true); // Fill
    }
    else
    {
        // Default view - title and basic info
        active_param = -1;
        
        hw.display.SetCursor(0, 0);
        hw.display.WriteString("DUAL OSC SYNTH", Font_6x8, true);
        
        sprintf(buf, "Oct: %d  %s", octave, osc1_wave < 0.5f ? "SAW" : "SQR");
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "Cut: %dHz  Res: %.0f%%", (int)filt_cutoff, (filt_res/0.95f)*100.f);
        hw.display.SetCursor(0, 24);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "Det: %+.1f  Mix: %.0f%%", osc2_detune, osc_mix*100.f);
        hw.display.SetCursor(0, 36);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "A: %.0fms  R: %.0fms", attack_time*1000.f, release_time*1000.f);
        hw.display.SetCursor(0, 48);
        hw.display.WriteString(buf, Font_6x8, true);
    }
    
    hw.display.Update();
}

void UpdateLeds()
{
    // Keyboard LED mapping (playable keys only - excludes gaps at 8, 11, 15)
    size_t keyboard_leds[] = {
        DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,  // Bottom row (0-7)
        DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2,
        DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
        DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8,  // Top row (8-15)
    };
    
    // Light all playable keyboard keys (always on), gap keys off
    for(size_t i = 0; i < 16; i++)
    {
        // Skip gap keys (not playable)
        if(i == 8 || i == 11 || i == 15)
        {
            hw.led_driver.SetLed(keyboard_leds[i], 0.f);
        }
        else
        {
            hw.led_driver.SetLed(keyboard_leds[i], 1.f);  // All playable keys lit
        }
    }
    
    // Knob LEDs show current values
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8,
    };
    
    float knob_vals[] = {osc1_wave, (osc2_detune + 12.f) / 24.f, osc_mix, 
                         filt_cutoff / 10000.f, filt_res / 0.95f, filt_env_amt,
                         attack_time / 2.f, release_time / 3.f};
    
    for(size_t i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], knob_vals[i]);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    
    // Octave control via switches (like stringvoice_overdrive_reverb)
    octave += hw.sw[0].RisingEdge() ? -1 : 0;
    octave += hw.sw[1].RisingEdge() ? 1 : 0;
    octave = DSY_MIN(DSY_MAX(0, octave), 4);
    
    // Keyboard input - handle note on/off
    for(size_t i = 0; i < 16; i++)
    {
        if(hw.KeyboardRisingEdge(i) && i != 8 && i != 11 && i != 15)
        {
            active_note = scale[i];
            active_key = i;
            gate = true;
        }
        if(hw.KeyboardFallingEdge(i) && i == active_key)
        {
            gate = false;
            active_key = 255;
        }
    }
    
    // Read knobs and detect changes
    float knob_raw[8];
    for(int k = 0; k < 8; k++)
    {
        knob_raw[k] = hw.knob[k].Process();
        if(fabsf(knob_raw[k] - prev_knobs[k]) > KNOB_THRESHOLD)
        {
            active_param = k;
            param_display_time = System::GetNow();
            prev_knobs[k] = knob_raw[k];
        }
    }
    
    // Map knob values to parameters
    osc1_wave    = knob_raw[0];                           // 0-1
    osc2_detune  = (knob_raw[1] - 0.5f) * 24.f;           // -12 to +12 semitones
    osc_mix      = knob_raw[2];                           // 0-1
    filt_cutoff  = 50.f + knob_raw[3] * 9950.f;           // 50-10000 Hz
    filt_res     = knob_raw[4] * 0.95f;                   // 0-0.95
    filt_env_amt = knob_raw[5];                           // 0-1
    attack_time  = 0.005f + knob_raw[6] * 1.995f;         // 5ms-2s
    release_time = 0.01f + knob_raw[7] * 2.99f;           // 10ms-3s
    
    // Update envelope times
    ampEnv.SetTime(ADSR_SEG_ATTACK, attack_time);
    ampEnv.SetTime(ADSR_SEG_RELEASE, release_time);
    filtEnv.SetTime(ADSR_SEG_ATTACK, attack_time);
    filtEnv.SetTime(ADSR_SEG_RELEASE, release_time);
    
    // Set oscillator waveform (morph saw->square)
    if(osc1_wave < 0.5f)
    {
        osc1.SetWaveform(Oscillator::WAVE_SAW);
        osc2.SetWaveform(Oscillator::WAVE_SAW);
    }
    else
    {
        osc1.SetWaveform(Oscillator::WAVE_SQUARE);
        osc2.SetWaveform(Oscillator::WAVE_SQUARE);
    }
    
    // Calculate note frequency
    float midi_note = 36.f + (octave * 12.f) + active_note;
    float freq1 = SemiToFreq(midi_note);
    float freq2 = SemiToFreq(midi_note + osc2_detune);
    
    osc1.SetFreq(freq1);
    osc2.SetFreq(freq2);
    
    // Set filter resonance
    filter.SetRes(filt_res);
    
    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        // Get oscillator outputs
        float sig1 = osc1.Process();
        float sig2 = osc2.Process();
        
        // Mix oscillators (linear crossfade)
        float sig = sig1 * (1.f - osc_mix) + sig2 * osc_mix;
        
        // Filter envelope modulation
        float filt_env = filtEnv.Process(gate);
        float cutoff_mod = filt_cutoff + (filt_env * filt_env_amt * 8000.f);
        cutoff_mod = fclamp(cutoff_mod, 20.f, 18000.f);
        filter.SetFreq(cutoff_mod);
        
        // Apply filter
        filter.Process(sig);
        sig = filter.Low();
        
        // Apply amplitude envelope
        float amp_env = ampEnv.Process(gate);
        sig *= amp_env;
        
        // Output to both channels
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void)
{
    hw.Init();
    float sample_rate = hw.AudioSampleRate();
    
    // Initialize oscillators
    osc1.Init(sample_rate);
    osc1.SetWaveform(Oscillator::WAVE_SAW);
    osc1.SetAmp(0.5f);
    
    osc2.Init(sample_rate);
    osc2.SetWaveform(Oscillator::WAVE_SAW);
    osc2.SetAmp(0.5f);
    
    // Initialize filter
    filter.Init(sample_rate);
    filter.SetFreq(2000.f);
    filter.SetRes(0.3f);
    
    // Initialize amplitude envelope
    ampEnv.Init(sample_rate);
    ampEnv.SetTime(ADSR_SEG_ATTACK, 0.01f);
    ampEnv.SetTime(ADSR_SEG_DECAY, 0.1f);
    ampEnv.SetTime(ADSR_SEG_RELEASE, 0.2f);
    ampEnv.SetSustainLevel(0.8f);
    
    // Initialize filter envelope
    filtEnv.Init(sample_rate);
    filtEnv.SetTime(ADSR_SEG_ATTACK, 0.01f);
    filtEnv.SetTime(ADSR_SEG_DECAY, 0.3f);
    filtEnv.SetTime(ADSR_SEG_RELEASE, 0.2f);
    filtEnv.SetSustainLevel(0.3f);
    
    // Start audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    // Main loop - update display and LEDs
    while(1)
    {
        UpdateOled();
        UpdateLeds();
        System::Delay(1);
    }
}
