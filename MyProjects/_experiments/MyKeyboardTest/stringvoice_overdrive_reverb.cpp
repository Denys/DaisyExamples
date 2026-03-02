/**
 * StringVoice with Overdrive and Reverb - Daisy Field Example
 * 
 * Based on: stringvoice_overdrive project
 * Added: ReverbSc from DSP-code-examples.txt -> reverbsc.cpp
 * 
 * Signal Chain: StringVoice -> Overdrive -> ReverbSc -> Output
 * 
 * Controls:
 *   Knob 1: Brightness (StringVoice)
 *   Knob 2: Structure (StringVoice)
 *   Knob 3: Damping (StringVoice)
 *   Knob 4: Accent (StringVoice)
 *   Knob 5: Overdrive Amount (0.0 = clean, 1.0 = full drive)
 *   Knob 6: Overdrive Dry/Wet Mix
 *   Knob 7: Reverb Feedback (decay time)
 *   Knob 8: Reverb Dry/Wet Mix
 *   SW1: Octave Down
 *   SW2: Octave Up
 *   Keyboard: Play notes (major scale + chromatic)
 */

#include "daisy_field.h"
#include "daisysp.h"

using namespace daisysp;
using namespace daisy;

#define NUM_CONTROLS 8

DaisyField  hw;
StringVoice str;
Overdrive   drive;
ReverbSc    verb;

uint8_t buttons[16];

// Major scale on bottom row, chromatic accidentals on top row
// Keys 8, 11, 15 are unused (black key gaps in layout)
float scale[16] = {
    0.f, 2.f, 4.f, 5.f, 7.f, 9.f, 11.f, 12.f,   // C D E F G A B C (bottom)
    0.f, 1.f, 3.f, 0.f, 6.f, 8.f, 10.f, 0.0f    // C# Eb - F# Ab Bb (top)
};

float active_note = scale[0];
int8_t octaves = 2;
float kvals[NUM_CONTROLS];

// Previous knob values for change detection
float prev_knobs[8] = {0.f};
const float KNOB_THRESHOLD = 0.01f;

// Active parameter display
int8_t   active_param = -1;
uint32_t param_display_time = 0;
const uint32_t PARAM_TIMEOUT_MS = 1000;

// Parameter names
const char* param_names[8] = {
    "BRIGHTNESS", "STRUCTURE", "DAMPING", "ACCENT",
    "DRIVE AMT", "DRIVE MIX", "VERB FB", "VERB MIX"
};

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    // Octave control via switches
    octaves += hw.sw[0].RisingEdge() ? -1 : 0;
    octaves += hw.sw[1].RisingEdge() ? 1 : 0;
    octaves = DSY_MIN(DSY_MAX(0, octaves), 4);

    // Read all 8 knob values and detect changes
    for(int i = 0; i < NUM_CONTROLS; i++)
    {
        kvals[i] = hw.GetKnobValue(i);
        if(fabsf(kvals[i] - prev_knobs[i]) > KNOB_THRESHOLD)
        {
            active_param = i;
            param_display_time = System::GetNow();
            prev_knobs[i] = kvals[i];
        }
    }

    // StringVoice parameters (Knobs 1-4)
    str.SetBrightness(kvals[0]);
    str.SetStructure(kvals[1]);
    str.SetDamping(kvals[2]);
    str.SetAccent(kvals[3]);

    // Overdrive parameters (Knobs 5-6)
    float driveAmount = kvals[4];
    float driveMix = kvals[5];
    drive.SetDrive(driveAmount);

    // Reverb parameters (Knobs 7-8)
    float reverbFeedback = 0.6f + kvals[6] * 0.35f;  // 0.6 to 0.95 range
    float reverbMix = kvals[7];
    verb.SetFeedback(reverbFeedback);
    verb.SetLpFreq(10000.0f);  // Fixed LP freq, could add another control

    // Keyboard input - trigger notes
    for(size_t i = 0; i < 16; i++)
    {
        if(hw.KeyboardRisingEdge(i) && i != 8 && i != 11 && i != 15)
        {
            str.Trig();
            float m = (12.0f * octaves) + 24.0f + scale[i];
            str.SetFreq(mtof(m));
        }
    }

    // Audio processing
    for(size_t i = 0; i < size; i++)
    {
        // 1. Get StringVoice output
        float dry = str.Process();
        
        // 2. Apply overdrive with mix
        float driven = drive.Process(dry);
        float afterDrive = dry * (1.0f - driveMix) + driven * driveMix;
        
        // 3. Apply reverb
        float wetL, wetR;
        verb.Process(afterDrive, afterDrive, &wetL, &wetR);
        
        // 4. Reverb dry/wet mix
        out[0][i] = afterDrive * (1.0f - reverbMix) + wetL * reverbMix;
        out[1][i] = afterDrive * (1.0f - reverbMix) + wetR * reverbMix;
    }
}

void UpdateLeds(float *knob_vals)
{
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1, DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3, DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5, DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7, DaisyField::LED_KNOB_8,
    };
    
    size_t keyboard_leds[] = {
        DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2,
        DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
        DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6,
        DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,
        DaisyField::LED_KEY_B2, DaisyField::LED_KEY_B3,
        DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6,
        DaisyField::LED_KEY_B7,
    };
    
    // Set all 8 knob LEDs to reflect current values
    for(size_t i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], knob_vals[i]);
    }
    
    // Light up playable keyboard keys
    for(size_t i = 0; i < 13; i++)
    {
        hw.led_driver.SetLed(keyboard_leds[i], 1.f);
    }
    
    hw.led_driver.SwapBuffersAndTransmit();
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
            case 0:  // Brightness
                sprintf(buf, "%.0f%%", kvals[0] * 100.f);
                break;
            case 1:  // Structure
                sprintf(buf, "%.0f%%", kvals[1] * 100.f);
                break;
            case 2:  // Damping
                sprintf(buf, "%.0f%%", kvals[2] * 100.f);
                break;
            case 3:  // Accent
                sprintf(buf, "%.0f%%", kvals[3] * 100.f);
                break;
            case 4:  // Drive Amount
                sprintf(buf, "%.0f%%", kvals[4] * 100.f);
                break;
            case 5:  // Drive Mix
                sprintf(buf, "%.0f%%", kvals[5] * 100.f);
                break;
            case 6:  // Reverb Feedback
                sprintf(buf, "%.0f%%", (0.6f + kvals[6] * 0.35f) * 100.f);
                break;
            case 7:  // Reverb Mix
                sprintf(buf, "%.0f%%", kvals[7] * 100.f);
                break;
        }
        hw.display.WriteString(buf, Font_11x18, true);
        
        // Progress bar
        int bar_width = (int)(kvals[active_param] * 120.f);
        hw.display.DrawRect(0, 50, 127, 58, true, false);
        hw.display.DrawRect(1, 51, bar_width, 57, true, true);
    }
    else
    {
        // Default view
        active_param = -1;
        
        hw.display.SetCursor(0, 0);
        hw.display.WriteString("STRING VOICE FX", Font_6x8, true);
        
        sprintf(buf, "Oct:%d Brt:%.0f%% Str:%.0f%%", octaves, kvals[0]*100.f, kvals[1]*100.f);
        hw.display.SetCursor(0, 12);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "Dmp:%.0f%% Acc:%.0f%%", kvals[2]*100.f, kvals[3]*100.f);
        hw.display.SetCursor(0, 24);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "Drv:%.0f%% Mix:%.0f%%", kvals[4]*100.f, kvals[5]*100.f);
        hw.display.SetCursor(0, 36);
        hw.display.WriteString(buf, Font_6x8, true);
        
        sprintf(buf, "VrbFB:%.0f%% VrbMx:%.0f%%", (0.6f+kvals[6]*0.35f)*100.f, kvals[7]*100.f);
        hw.display.SetCursor(0, 48);
        hw.display.WriteString(buf, Font_6x8, true);
    }
    
    hw.display.Update();
}

int main(void)
{
    hw.Init();
    float sr = hw.AudioSampleRate();

    // Initialize StringVoice (physical modeling)
    octaves = 2;
    str.Init(sr);

    // Initialize Overdrive
    drive.Init();

    // Initialize Reverb
    verb.Init(sr);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(10000.0f);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    for(;;)
    {
        UpdateLeds(kvals);
        UpdateOled();
        System::Delay(6);
    }
}
