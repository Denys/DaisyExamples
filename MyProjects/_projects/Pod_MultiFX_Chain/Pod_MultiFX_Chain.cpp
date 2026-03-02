/**
 * Pod_MultiFX_Chain
 *
 * 3-stage serial FX chain: Overdrive → Delay → Reverb
 * with switchable parameter control.
 *
 * Platform: Daisy Pod
 * Complexity: ★★★★★★☆☆
 *
 * Controls:
 * - Button 1: Cycle active FX (Overdrive/Delay/Reverb)
 * - Knob 1: Edit selected FX parameter 1
 *   - Overdrive: Drive amount
 *   - Delay: Time
 *   - Reverb: Feedback/Decay
 * - Knob 2: Edit selected FX parameter 2
 *   - Overdrive: Tone/Filter (not used - OD has no tone param)
 *   - Delay: Feedback
 *   - Reverb: LP Frequency/Damping
 * - Button 2: Bypass current FX stage
 * - LED: Color indicates active FX (Red=OD, Green=Delay, Blue=Reverb)
 */

#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// FX Modules
Overdrive overdrive;
DelayLine<float, 48000> DSY_SDRAM_BSS delayLine; // 1 second max delay at 48kHz
ReverbSc reverb;

// FX Selection
enum FxSelect {
    FX_OVERDRIVE = 0,
    FX_DELAY = 1,
    FX_REVERB = 2,
    FX_COUNT = 3
};

FxSelect currentFx = FX_OVERDRIVE;

// FX Parameters
float od_drive = 0.5f;
float delay_time = 0.3f;      // 300ms
float delay_feedback = 0.5f;
float reverb_feedback = 0.85f;
float reverb_lpfreq = 10000.0f;

// Bypass states
bool od_bypassed = false;
bool delay_bypassed = false;
bool reverb_bypassed = false;

// Control state
float delayBuffer = 0.0f;

void UpdateControls();
void UpdateLED();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls();

    for(size_t i = 0; i < size; i++)
    {
        float input = in[0][i]; // Mono input
        float signal = input;

        // Stage 1: Overdrive
        if(!od_bypassed)
        {
            signal = overdrive.Process(signal);
        }

        // Stage 2: Delay
        if(!delay_bypassed)
        {
            // Read delayed signal
            delayBuffer = delayLine.Read();

            // Mix dry + delayed
            signal = signal + delayBuffer * delay_feedback;

            // Write to delay line
            delayLine.Write(signal);
        }

        // Stage 3: Reverb (Stereo)
        float wetL, wetR;
        if(!reverb_bypassed)
        {
            reverb.Process(signal, signal, &wetL, &wetR);
            out[0][i] = wetL;
            out[1][i] = wetR;
        }
        else
        {
            out[0][i] = signal;
            out[1][i] = signal;
        }
    }
}

void UpdateControls()
{
    hw.ProcessAllControls();

    // Button 1: Cycle through FX
    if(hw.button1.RisingEdge())
    {
        currentFx = static_cast<FxSelect>((currentFx + 1) % FX_COUNT);
    }

    // Button 2: Bypass current FX
    if(hw.button2.RisingEdge())
    {
        switch(currentFx)
        {
            case FX_OVERDRIVE:
                od_bypassed = !od_bypassed;
                break;
            case FX_DELAY:
                delay_bypassed = !delay_bypassed;
                break;
            case FX_REVERB:
                reverb_bypassed = !reverb_bypassed;
                break;
        }
    }

    // Knob 1 & 2: Edit selected FX parameters
    float knob1 = hw.knob1.Process();
    float knob2 = hw.knob2.Process();

    switch(currentFx)
    {
        case FX_OVERDRIVE:
            // K1: Drive (0-1)
            od_drive = knob1;
            overdrive.SetDrive(od_drive);
            // K2: Not used (Overdrive has no tone parameter)
            break;

        case FX_DELAY:
            // K1: Time (10ms - 1000ms)
            delay_time = 0.01f + knob1 * 0.99f;
            delayLine.SetDelay(hw.AudioSampleRate() * delay_time);

            // K2: Feedback (0-95%)
            delay_feedback = knob2 * 0.95f;
            break;

        case FX_REVERB:
            // K1: Feedback/Decay (0.5 - 0.99)
            reverb_feedback = 0.5f + knob1 * 0.49f;
            reverb.SetFeedback(reverb_feedback);

            // K2: LP Frequency/Damping (1000Hz - 18000Hz)
            reverb_lpfreq = 1000.0f + knob2 * 17000.0f;
            reverb.SetLpFreq(reverb_lpfreq);
            break;
    }

    UpdateLED();
}

void UpdateLED()
{
    float brightness = 1.0f;

    // Dim if bypassed
    switch(currentFx)
    {
        case FX_OVERDRIVE:
            if(od_bypassed) brightness = 0.1f;
            break;
        case FX_DELAY:
            if(delay_bypassed) brightness = 0.1f;
            break;
        case FX_REVERB:
            if(reverb_bypassed) brightness = 0.1f;
            break;
    }

    // Set color based on active FX
    switch(currentFx)
    {
        case FX_OVERDRIVE:
            // Red
            hw.led1.SetRed(brightness);
            hw.led1.SetGreen(0.0f);
            hw.led1.SetBlue(0.0f);
            break;

        case FX_DELAY:
            // Green
            hw.led1.SetRed(0.0f);
            hw.led1.SetGreen(brightness);
            hw.led1.SetBlue(0.0f);
            break;

        case FX_REVERB:
            // Blue
            hw.led1.SetRed(0.0f);
            hw.led1.SetGreen(0.0f);
            hw.led1.SetBlue(brightness);
            break;
    }

    hw.led1.Update();
}

int main(void)
{
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(48);

    float sampleRate = hw.AudioSampleRate();

    // Initialize Overdrive
    overdrive.Init();
    overdrive.SetDrive(od_drive);

    // Initialize Delay
    delayLine.Init();
    delayLine.SetDelay(sampleRate * delay_time);

    // Initialize Reverb
    reverb.Init(sampleRate);
    reverb.SetFeedback(reverb_feedback);
    reverb.SetLpFreq(reverb_lpfreq);

    // Start audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Initial LED update
    UpdateLED();

    // Main loop
    while(1)
    {
        // LED updates happen in AudioCallback via UpdateControls
    }
}
