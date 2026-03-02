#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
Tube      tube;
Oscillator osc;
bool      effect_on;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Debounce button
    
    // Simple toggle on button press (assuming button on pin 28 for test, or user button)
    // Here we just rely on effect_on state toggled in main or hardcoded.
    // For simplicity, let's just make it always on for this basic test, 
    // or toggle with a delay in main loop if we had a button class instance.
    
    for(size_t i = 0; i < size; i++)
    {
        float sig = osc.Process();
        float processed = tube.Process(sig);
        
        // Output logic
        out[0][i] = processed;
        out[1][i] = processed;
    }
}

int main(void)
{
    hw.Init();
    float sample_rate = hw.AudioSampleRate();

    // Init Tube
    tube.Init(sample_rate);
    tube.SetGain(6.0f);        // Drive
    tube.SetWorkPoint(-0.2f);  // Warm triode
    tube.SetDistortion(8.0f);  // Soft saturation
    tube.SetMix(0.5f);         // 50% wet

    // Init Test Oscillator
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetFreq(220.0f);
    osc.SetAmp(0.5f);

    hw.StartAudio(AudioCallback);

    while(1) 
    {
        // Blink LED to show life
        hw.SetLed(true);
        System::Delay(500);
        hw.SetLed(false);
        System::Delay(500);
    }
}
