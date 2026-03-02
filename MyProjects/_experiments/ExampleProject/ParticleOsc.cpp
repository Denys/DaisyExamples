/**
 * ParticleOsc.cpp - Particle Oscillator with SVF Filter
 * 
 * Monophonic synth for Daisy Field with:
 * - Particle Oscillator (granular noise synthesis)
 * - State Variable Filter (SVF) - lowpass mode
 * - Monophonic keyboard: C3-C4 mapped to KEY_B1-B8
 * - 6 knobs mapped to Particle and Filter parameters
 * 
 * Knob Mapping (as per diagram):
 * K0: Particle Density (DENS CV)
 * K1: Particle Spread (SPRE CV)
 * K2: Particle Frequency (FREQ CV)
 * K3: Particle Resonance (RES CV)
 * K4: Filter Cutoff (FREQ CV)
 * K5: Filter Resonance (RES CV)
 */

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Hardware
DaisyField hw;

// DSP Objects
Particle   particle;
Svf        filt;    // State Variable Filter (use Low() output for lowpass)
Adsr       env;

// State
float sample_rate;
bool  gate = false;
float currentFreq = 261.63f;  // C4 default

// MIDI note frequencies for C3-C4 scale mapped to KEY_B1-B8
const float noteFreqs[8] = {
    130.81f,  // C3  - KEY_B1 (index 0)
    146.83f,  // D3  - KEY_B2 (index 1)
    164.81f,  // E3  - KEY_B3 (index 2)
    174.61f,  // F3  - KEY_B4 (index 3)
    196.00f,  // G3  - KEY_B5 (index 4)
    220.00f,  // A3  - KEY_B6 (index 5)
    246.94f,  // B3  - KEY_B7 (index 6)
    261.63f   // C4  - KEY_B8 (index 7)
};

// Track previous key states for edge detection
bool prevKeyState[8] = {false};

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    hw.ProcessAllControls();
    
    // Read 6 knobs (K0-K5)
    float k0_density  = hw.knob[0].Value();  // K0: Particle Density (0.0-1.0)
    float k1_spread   = hw.knob[1].Value();  // K1: Particle Spread (0.0-1.0)
    float k2_freq     = hw.knob[2].Value();  // K2: Particle Freq mult
    float k3_res      = hw.knob[3].Value();  // K3: Particle Resonance
    float k4_cutoff   = hw.knob[4].Value();  // K4: Filter Cutoff
    float k5_filterRes = hw.knob[5].Value(); // K5: Filter Resonance
    
    // Scan keyboard for monophonic note selection (last-note priority)
    gate = false;
    for(int i = 0; i < 8; i++)
    {
        bool keyState = hw.KeyboardState(i);
        
        // Rising edge detection - new key pressed
        if(keyState && !prevKeyState[i])
        {
            currentFreq = noteFreqs[i];
            gate = true;
        }
        
        // Any key held keeps gate open
        if(keyState)
        {
            gate = true;
        }
        
        prevKeyState[i] = keyState;
    }
    
    // Update Particle parameters
    particle.SetFreq(currentFreq);
    particle.SetDensity(k0_density);                     // K0: 0.0-1.0
    particle.SetSpread(k1_spread);                       // K1: 0.0-1.0
    particle.SetRandomFreq(k2_freq * 10000.0f);          // K2: 0-10kHz range
    particle.SetResonance(k3_res);                       // K3: 0.0-1.0
    
    // Update SVF filter parameters (lowpass mode)
    // Cutoff: exponential mapping 20Hz - 20kHz
    float cutoffHz = 20.0f * powf(1000.0f, k4_cutoff);   // K4: 20Hz-20kHz
    filt.SetFreq(cutoffHz);
    filt.SetRes(k5_filterRes * 0.95f);                   // K5: 0.0-0.95 (safety limit)
    
    // Audio processing loop
    for(size_t i = 0; i < size; i++)
    {
        // Process envelope with gate
        float envVal = env.Process(gate);
        
        // Get particle oscillator output
        float particleOut = particle.Process();
        
        // Apply amplitude envelope
        particleOut *= envVal;
        
        // Apply SVF filter (use lowpass output like MoogLadder)
        filt.Process(particleOut);
        float filtered = filt.Low();  // Lowpass output
        
        // Output to both channels (stereo)
        out[0][i] = filtered;
        out[1][i] = filtered;
    }
}

int main(void)
{
    // Initialize Field hardware
    hw.Init();
    hw.SetAudioBlockSize(48);
    sample_rate = hw.AudioSampleRate();
    
    // Initialize Particle oscillator
    particle.Init(sample_rate);
    particle.SetFreq(currentFreq);
    particle.SetDensity(0.5f);        // Dens: 0.50 (as shown in diagram)
    particle.SetSpread(0.5f);         // Spre: 0.50 (as shown in diagram)
    particle.SetRandomFreq(5000.0f);
    particle.SetResonance(0.4f);
    particle.SetSync(true);
    
    // Initialize SVF filter (lowpass mode)
    filt.Init(sample_rate);
    filt.SetFreq(1000.0f);            // Cuto: 1000Hz (as shown in diagram)
    filt.SetRes(0.4f);                // Reso: 0.40 (as shown in diagram)
    
    // Initialize ADSR envelope
    env.Init(sample_rate);
    env.SetTime(ADSR_SEG_ATTACK, 0.005f);   // 5ms attack
    env.SetTime(ADSR_SEG_DECAY, 0.2f);      // 200ms decay
    env.SetSustainLevel(0.7f);              // 70% sustain
    env.SetTime(ADSR_SEG_RELEASE, 0.3f);    // 300ms release
    
    // Start audio and ADC
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    // Main loop - LED feedback
    while(1)
    {
        // Light up LED corresponding to pressed key
        for(int i = 0; i < 8; i++)
        {
            float brightness = hw.KeyboardState(i) ? 1.0f : 0.1f;
            hw.led_driver.SetLed(i, brightness);
        }
        hw.led_driver.SwapBuffersAndTransmit();
        
        System::Delay(10);
    }
}
