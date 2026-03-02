#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

// Hardware and UI
DaisyField        hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay  display;

// Synthesis engines
const int  kNumModes = 8;
ModalVoice modal[kNumModes];
// ReverbSc   reverb;  // TODO: Add reverb back with proper LGPL include

// State
bool mode_active[kNumModes] = {false,
                               true,
                               false,
                               true,
                               false,
                               false,
                               false,
                               false}; // Start with some active
int  strike_type            = 0;       // 0-7 for different strike types
bool compact_view           = false;

// Parameters
struct Params
{
    float brightness   = 0.5f;
    float structure    = 0.5f;
    float damping      = 0.5f;
    float accent       = 0.8f;
    float reverb_size  = 0.7f;
    float reverb_decay = 0.7f;
    float dry_wet      = 0.4f;
    float master_level = 0.8f;
} params;

// Modal frequencies (bell harmonic series in Hz for C4 base)
const float kModeFreqs[kNumModes] = {
    261.63f, // C4  - Fundamental
    392.00f, // G4  - Perfect 5th
    523.25f, // C5  - Octave
    659.25f, // E5  - Major 3rd
    783.99f, // G5  - Perfect 5th
    880.00f, // A5  - Major 6th
    1046.5f, // C6  - 2 octaves
    1318.5f  // E6  - Major 3rd up
};

// Strike characteristics
struct StrikeParams
{
    float       velocity;
    float       brightness_mod;
    const char* name;
};

const StrikeParams kStrikeTypes[8] = {
    {0.3f, 0.2f, "Soft"},   // B1
    {0.5f, 0.4f, "Medium"}, // B2
    {0.8f, 0.7f, "Hard"},   // B3
    {1.0f, 1.0f, "Metal"},  // B4
    {0.4f, 0.1f, "Brush"},  // B5
    {0.7f, 0.9f, "Mallet"}, // B6
    {0.6f, 0.5f, "Felt"},   // B7
    {0.9f, 0.6f, "Wood"}    // B8
};

void ProcessKnobs()
{
    // Read parameters from knobs
    params.brightness   = hw.knob[0].Process();
    params.structure    = hw.knob[1].Process();
    params.damping      = hw.knob[2].Process();
    params.accent       = hw.knob[3].Process();
    params.reverb_size  = hw.knob[4].Process();
    params.reverb_decay = hw.knob[5].Process();
    params.dry_wet      = hw.knob[6].Process();
    params.master_level = hw.knob[7].Process();

    // Update modal synthesis parameters
    for(int i = 0; i < kNumModes; i++)
    {
        modal[i].SetFreq(kModeFreqs[i]);
        modal[i].SetBrightness(params.brightness);
        modal[i].SetStructure(params.structure);
        modal[i].SetDamping(params.damping);
        modal[i].SetAccent(params.accent);
    }

    // Update reverb
    // reverb.SetFeedback(params.reverb_size);
    // reverb.SetLpFreq(10000.0f * (1.0f - params.reverb_decay));
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Audio processing only - NO control processing here!
    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        float sig = 0.0f;

        // Sum active modal voices
        for(int m = 0; m < kNumModes; m++)
        {
            if(mode_active[m])
                sig += modal[m].Process();
        }

        // Reverb processing (disabled)
        // float rev_l, rev_r;
        // reverb.Process(sig, sig, &rev_l, &rev_r);

        // Dry/wet mix (no reverb)
        // float mix = params.dry_wet;
        out[0][i]
            = sig
              * params
                    .master_level; // (sig * (1.0f - mix) + rev_l * mix) * params.master_level;
        out[1][i]
            = sig
              * params
                    .master_level; // (sig * (1.0f - mix) + rev_r * mix) * params.master_level;
    }
}

int main(void)
{
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    float sr = hw.AudioSampleRate();

    // Initialize UI helpers
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Modal Bells");

    // Set parameter labels
    display.SetLabel(0, "Bright");
    display.SetLabel(1, "Struct");
    display.SetLabel(2, "Damping");
    display.SetLabel(3, "Accent");
    display.SetLabel(4, "RevSize");
    display.SetLabel(5, "RevDcy");
    display.SetLabel(6, "Mix");
    display.SetLabel(7, "Level");

    // Initialize modal voices
    for(int i = 0; i < kNumModes; i++)
    {
        modal[i].Init(sr);
        modal[i].SetFreq(kModeFreqs[i]);
        modal[i].SetBrightness(0.5f);
        modal[i].SetStructure(0.5f);
        modal[i].SetDamping(0.5f);
        modal[i].SetAccent(0.8f);
    }

    // Initialize reverb (disabled)
    // reverb.Init(sr);
    // reverb.SetFeedback(0.7f);
    // reverb.SetLpFreq(8000.0f);

    // Set initial LED states for active modes
    for(int i = 0; i < kNumModes; i++)
        keyLeds.SetA(i, mode_active[i]);

    // Set initial strike type LED
    keyLeds.SetB(strike_type, true);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        // Process controls in main loop (NOT in audio callback!)
        hw.ProcessAllControls();
        ProcessKnobs();

        // Row A: Play modal voices (always trigger on press)
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyAIndices[i]))
            {
                // Toggle LED state
                keyLeds.ToggleA(i);
                mode_active[i] = !mode_active[i];

                // ALWAYS trigger on key press (not just when turning on)
                float velocity   = kStrikeTypes[strike_type].velocity;
                float bright_mod = kStrikeTypes[strike_type].brightness_mod;
                modal[i].SetBrightness(params.brightness * bright_mod);
                modal[i].SetAccent(velocity);
                modal[i].Trig();
            }
        }

        // Row B: Select strike type
        for(int i = 0; i < 8; i++)
        {
            if(hw.KeyboardRisingEdge(kKeyBIndices[i]))
            {
                // Clear all B LEDs first
                for(int j = 0; j < 8; j++)
                    keyLeds.SetB(j, false);

                // Set new strike type
                strike_type = i;
                keyLeds.SetB(i, true);

                // DEBUG TEST: Disable Trig() to test if it causes crash
                // Trigger all active modes with new strike type
                // for(int m = 0; m < kNumModes; m++)
                // {
                //     if(mode_active[m])
                //     {
                //         modal[m].SetBrightness(
                //             params.brightness * kStrikeTypes[i].brightness_mod);
                //         modal[m].SetAccent(kStrikeTypes[i].velocity);
                //         modal[m].Trig();
                //     }
                // }
            }
        }

        // SW1: Toggle compact view
        if(hw.sw[0].RisingEdge())
            compact_view = !compact_view;

        // SW2: Trigger all active modes
        if(hw.sw[1].RisingEdge())
        {
            for(int i = 0; i < kNumModes; i++)
            {
                if(mode_active[i])
                {
                    modal[i].SetBrightness(
                        params.brightness
                        * kStrikeTypes[strike_type].brightness_mod);
                    modal[i].SetAccent(kStrikeTypes[strike_type].velocity);
                    modal[i].Trig();
                }
            }
        }

        // Update keyboard LEDs
        keyLeds.Update();

        // Update display with parameter values
        for(int i = 0; i < 8; i++)
            display.SetValue(i, hw.knob[i].Process());

        // Clear display and show custom info line
        hw.display.Fill(false);

        // Title with mode count
        char title_buf[32];
        int  active_count = 0;
        for(int i = 0; i < kNumModes; i++)
            if(mode_active[i])
                active_count++;

        snprintf(title_buf,
                 sizeof(title_buf),
                 "ModalBells %dM %s",
                 active_count,
                 kStrikeTypes[strike_type].name);
        hw.display.SetCursor(0, 0);
        hw.display.WriteString(title_buf, Font_7x10, true);

        // Show parameter display
        if(compact_view)
        {
            // Compact: all 8 params in 2 columns
            const char* labels[8]
                = {"Bri", "Str", "Dmp", "Acc", "Siz", "Dcy", "Mix", "Lvl"};
            int y = 12;
            for(int i = 0; i < 8; i++)
            {
                char buf[12];
                snprintf(buf,
                         sizeof(buf),
                         "%s:%.2f",
                         labels[i],
                         hw.knob[i].Process());
                int x   = (i < 4) ? 0 : 64;
                int row = i % 4;
                hw.display.SetCursor(x, y + (row * 13));
                hw.display.WriteString(buf, Font_6x8, true);
            }
        }
        else
        {
            // Standard: show last changed param large
            int active_param = display.GetActiveParam();
            if(active_param >= 0)
            {
                char        buf[32];
                const char* labels[8] = {"Brightness",
                                         "Structure",
                                         "Damping",
                                         "Accent",
                                         "Rev Size",
                                         "Rev Decay",
                                         "Dry/Wet",
                                         "Level"};

                hw.display.SetCursor(0, 12);
                snprintf(buf, sizeof(buf), "> %s", labels[active_param]);
                hw.display.WriteString(buf, Font_7x10, true);

                hw.display.SetCursor(0, 24);
                snprintf(
                    buf, sizeof(buf), "%.2f", hw.knob[active_param].Process());
                hw.display.WriteString(buf, Font_11x18, true);

                // Show first 2 params small
                hw.display.SetCursor(0, 46);
                hw.display.WriteString("---", Font_6x8, true);
                for(int i = 0; i < 2; i++)
                {
                    snprintf(buf, sizeof(buf), "%.2f", hw.knob[i].Process());
                    hw.display.SetCursor((i * 32), 54);
                    hw.display.WriteString(buf, Font_6x8, true);
                }
            }
        }

        hw.display.Update();

        // Update knob LEDs to match values
        for(int i = 0; i < 8; i++)
            hw.led_driver.SetLed(kLedKnobs[i], hw.knob[i].Value());

        // Update switch LEDs
        hw.led_driver.SetLed(kLedSwitches[0], compact_view ? 1.0f : 0.0f);
        hw.led_driver.SetLed(kLedSwitches[1], 0.0f);

        // Transmit all LED updates (knob and switch LEDs set after keyLeds.Update())
        hw.led_driver.SwapBuffersAndTransmit();

        System::Delay(16);
    }
}
