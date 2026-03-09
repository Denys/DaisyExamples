/**
 * StringVoice with Overdrive and Reverb - Daisy Field Example
 * 
 * Based on: stringvoice_overdrive project
 * Added: ReverbSc from DSP-code-examples.txt -> reverbsc.cpp
 * 
 * Signal Chain: StringVoice -> Overdrive -> Compressor -> ReverbSc -> Output
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
#include <cstdint>

using namespace daisysp;
using namespace daisy;

#define NUM_CONTROLS 8

DaisyField  hw;
StringVoice str;
Overdrive   drive;
Compressor  comp;
ReverbSc    verb;

uint8_t buttons[16];

// Major scale on bottom row, chromatic accidentals on top row
// Keys 8, 11, 15 are unused (black key gaps in layout)
float scale[16] = {
    0.f,
    2.f,
    4.f,
    5.f,
    7.f,
    9.f,
    11.f,
    12.f, // C D E F G A B C (bottom)
    0.f,
    1.f,
    3.f,
    0.f,
    6.f,
    8.f,
    10.f,
    0.0f // C# Eb - F# Ab Bb (top)
};

float  active_note = scale[0];
int8_t octaves     = 2;
float  kvals[NUM_CONTROLS];
float  ksmooth[NUM_CONTROLS]; // Smoothed knob values for anti-zipper

// OLED Zoom Visualization
float    prevKnob[NUM_CONTROLS];
int      zoomParam     = -1;
uint32_t zoomStartTime = 0;

// MIDI State
float g_midi_freq    = 0.0f;
bool  g_midi_trigger = false;

void HandleMidi()
{
    hw.midi.Listen();
    while(hw.midi.HasEvents())
    {
        MidiEvent me = hw.midi.PopEvent();
        if(me.type == NoteOn)
        {
            NoteOnEvent p = me.AsNoteOn();
            if(p.velocity > 0)
            {
                g_midi_freq    = mtof(p.note);
                g_midi_trigger = true;
            }
        }
    }
}

const char *paramNames[NUM_CONTROLS] = {"Bright",
                                        "Struct",
                                        "Damp",
                                        "Accent",
                                        "Drive",
                                        "DrvMix",
                                        "RvbFB",
                                        "RvbMix"};

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();

    // Octave control via switches
    octaves += hw.sw[0].RisingEdge() ? -1 : 0;
    octaves += hw.sw[1].RisingEdge() ? 1 : 0;
    octaves = DSY_MIN(DSY_MAX(0, octaves), 4);

    // Read all 8 knob values
    for(int i = 0; i < NUM_CONTROLS; i++)
    {
        kvals[i] = hw.GetKnobValue(i);
    }

    // Apply parameter smoothing (fonepole) to prevent zipper noise
    for(int i = 0; i < NUM_CONTROLS; i++)
    {
        fonepole(ksmooth[i], kvals[i], .0001f);
    }

    // StringVoice parameters (Knobs 1-4) - use smoothed values
    str.SetBrightness(ksmooth[0]);
    str.SetStructure(ksmooth[1]);
    str.SetDamping(ksmooth[2]);
    str.SetAccent(ksmooth[3]);

    // Overdrive parameters (Knobs 5-6) - use smoothed values
    float driveAmount = ksmooth[4];
    float driveMix    = ksmooth[5];
    drive.SetDrive(driveAmount);

    // Reverb parameters (Knobs 7-8) - use smoothed values
    float reverbFeedback = 0.6f + ksmooth[6] * 0.35f; // 0.6 to 0.95 range
    float reverbMix      = ksmooth[7];
    verb.SetFeedback(reverbFeedback);
    verb.SetLpFreq(10000.0f); // Fixed LP freq, could add another control

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

    // Check for MIDI Triggers
    if(g_midi_trigger)
    {
        str.SetFreq(g_midi_freq);
        str.Trig();
        g_midi_trigger = false;
    }

    // Audio processing
    for(size_t i = 0; i < size; i++)
    {
        // 1. Get StringVoice output
        float dry = str.Process();

        // 2. Apply overdrive with mix
        float driven     = drive.Process(dry);
        float afterDrive = dry * (1.0f - driveMix) + driven * driveMix;

        // 3. Apply Compressor
        float compressed = comp.Process(afterDrive);

        // 4. Apply reverb
        float wetL, wetR;
        verb.Process(compressed, compressed, &wetL, &wetR);

        // 5. Reverb dry/wet mix
        out[0][i] = compressed * (1.0f - reverbMix) + wetL * reverbMix;
        out[1][i] = compressed * (1.0f - reverbMix) + wetR * reverbMix;
    }
}

void UpdateLeds(float *knob_vals)
{
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1,
        DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3,
        DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5,
        DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7,
        DaisyField::LED_KNOB_8,
    };

    size_t keyboard_leds[] = {
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

int main(void)
{
    hw.Init();
    float sr = hw.AudioSampleRate();

    // Initialize StringVoice (physical modeling)
    octaves = 2;
    str.Init(sr);

    // Initialize Overdrive
    drive.Init();

    // Initialize Compressor
    comp.Init(sr);
    comp.SetThreshold(-12.0f);
    comp.SetRatio(4.0f);
    comp.SetAttack(0.005f);
    comp.SetRelease(0.100f);

    // Initialize Reverb
    verb.Init(sr);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(10000.0f);

    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    for(;;)
    {
        UpdateLeds(kvals);
        HandleMidi();

        // Check for parameter changes (OLED Zoom Visualization)
        for(int i = 0; i < NUM_CONTROLS; i++)
        {
            if(fabsf(kvals[i] - prevKnob[i]) > 0.02f)
            {
                zoomParam     = i;
                zoomStartTime = System::GetNow();
                prevKnob[i]   = kvals[i];
            }
        }
        if(System::GetNow() - zoomStartTime > 1200)
        {
            zoomParam = -1;
        }

        // Update display
        hw.display.Fill(false);
        char buf[32];

        if(zoomParam >= 0)
        {
            // Zoomed single-parameter view
            hw.display.SetCursor(0, 0);
            hw.display.WriteString(paramNames[zoomParam], Font_11x18, true);

            hw.display.SetCursor(0, 24);
            float val     = kvals[zoomParam];
            int   percent = (int)(val * 100.f);

            // Format value with context based on parameter
            if(zoomParam == 6) // Reverb Feedback
            {
                float mapped = 0.6f + val * 0.35f;
                sprintf(buf, "%d%% (%.2f)", percent, mapped);
            }
            else if(zoomParam == 5 || zoomParam == 7) // Dry/Wet Mixes
            {
                sprintf(buf, "%d%% (%.2f)", percent, val);
            }
            else // StringVoice Params (Abstract 0-1) and Drive
            {
                sprintf(buf, "%d%% (%.2f)", percent, val);
            }
            hw.display.WriteString(buf, Font_11x18, true);

            // Progress bar
            int barWidth = (int)(val * 120.f);
            hw.display.DrawRect(0, 50, 127, 58, true, false);         // Outline
            hw.display.DrawRect(1, 51, 1 + barWidth, 57, true, true); // Fill
        }
        else
        {
            // Normal overview display
            hw.display.SetCursor(0, 0);
            sprintf(buf, "Oct:%d Drv:%.0f%%", octaves, kvals[4] * 100.f);
            hw.display.WriteString(buf, Font_6x8, true);

            hw.display.SetCursor(0, 10);
            sprintf(buf,
                    "DrvMx:%.0f%% RvbMx:%.0f%%",
                    kvals[5] * 100.f,
                    kvals[7] * 100.f);
            hw.display.WriteString(buf, Font_6x8, true);

            hw.display.SetCursor(0, 20);
            sprintf(buf, "Reverb FB:%.0f%%", (0.6f + kvals[6] * 0.35f) * 100.f);
            hw.display.WriteString(buf, Font_6x8, true);

            hw.display.SetCursor(0, 32);
            sprintf(buf,
                    "Br:%.0f St:%.0f Da:%.0f Ac:%.0f",
                    kvals[0] * 100.f,
                    kvals[1] * 100.f,
                    kvals[2] * 100.f,
                    kvals[3] * 100.f);
            hw.display.WriteString(buf, Font_6x8, true);
        }

        hw.display.Update();

        System::Delay(6);
    }
}
