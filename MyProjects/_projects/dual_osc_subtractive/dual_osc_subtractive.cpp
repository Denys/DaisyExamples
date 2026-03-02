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
Crossfade  oscMix;

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

// Convert semitone to frequency
float SemiToFreq(float semi)
{
    return 440.f * powf(2.f, (semi - 69.f) / 12.f);
}

void UpdateOled()
{
    hw.display.Fill(false);
    
    // Title
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("DUAL OSC SYNTH", Font_6x8, true);
    
    // Octave
    char buf[16];
    sprintf(buf, "Oct: %d", octave);
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(buf, Font_6x8, true);
    
    // Filter cutoff
    sprintf(buf, "Filt: %dHz", (int)filt_cutoff);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(buf, Font_6x8, true);
    
    // Detune
    sprintf(buf, "Det: %.1f", osc2_detune);
    hw.display.SetCursor(0, 36);
    hw.display.WriteString(buf, Font_6x8, true);
    
    hw.display.Update();
}

void UpdateLeds()
{
    // Clear all LEDs
    for(int i = 0; i < 16; i++)
    {
        hw.led_driver.SetLed(i, 0.f);
    }
    
    // Light active key
    if(active_key < 16)
    {
        hw.led_driver.SetLed(active_key, 1.f);
    }
    
    // Octave indicator on LEDs 16-20
    for(int i = 0; i < 5; i++)
    {
        hw.led_driver.SetLed(16 + i, (i == octave) ? 1.f : 0.1f);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();
    
    // Read knobs
    osc1_wave    = hw.knob[0].Process();                        // 0-1
    osc2_detune  = (hw.knob[1].Process() - 0.5f) * 24.f;        // -12 to +12 semitones
    osc_mix      = hw.knob[2].Process();                        // 0-1
    filt_cutoff  = 50.f + hw.knob[3].Process() * 9950.f;        // 50-10000 Hz
    filt_res     = hw.knob[4].Process() * 0.95f;                // 0-0.95
    filt_env_amt = hw.knob[5].Process();                        // 0-1
    attack_time  = 0.005f + hw.knob[6].Process() * 1.995f;      // 5ms-2s
    release_time = 0.01f + hw.knob[7].Process() * 2.99f;        // 10ms-3s
    
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
    
    // Set crossfade position
    oscMix.SetPos(osc_mix);
    
    // Set filter resonance
    filter.SetRes(filt_res);
    
    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        // Get oscillator outputs
        float sig1 = osc1.Process();
        float sig2 = osc2.Process();
        
        // Mix oscillators
        float sig = oscMix.Process(sig1, sig2);
        
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
    
    // Initialize crossfade
    oscMix.Init();
    
    // Start audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    // Main loop - handle keyboard, switches, display
    while(1)
    {
        hw.ProcessDigitalControls();
        
        // Handle octave switches
        if(hw.sw[0].RisingEdge() && octave > 0)
        {
            octave--;
        }
        if(hw.sw[1].RisingEdge() && octave < 4)
        {
            octave++;
        }
        
        // Scan keyboard
        bool any_key_pressed = false;
        for(int i = 0; i < 16; i++)
        {
            if(hw.KeyboardRisingEdge(i))
            {
                // Skip gap keys (8, 11, 15)
                if(i == 8 || i == 11 || i == 15) continue;
                
                active_note = scale[i];
                active_key = i;
                gate = true;
                any_key_pressed = true;
            }
            
            if(hw.KeyboardFallingEdge(i))
            {
                if(i == active_key)
                {
                    gate = false;
                    active_key = 255;
                }
            }
            
            if(hw.KeyboardState(i))
            {
                any_key_pressed = true;
            }
        }
        
        // Update display and LEDs
        UpdateOled();
        UpdateLeds();
        
        System::Delay(1);
    }
}
