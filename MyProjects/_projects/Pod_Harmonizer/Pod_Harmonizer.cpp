/**
 * Pod_Harmonizer v1.0 - Daisy Pod
 * 
 * A stereo harmonizer effect with two pitch-shifted voices.
 * 
 * GENERATED FROM BLOCK DIAGRAM (Block-First Methodology)
 * See README.md for the source Mermaid diagram.
 * 
 * Controls:
 * Knob 1: Dry/Wet Mix (0-100%)
 * Knob 2: Detune Amount (0-50 cents)
 * Button 1: Cycle Harmony Preset (3rds / 5ths / Octave)
 * Button 2: Bypass Toggle
 * LED: Green=3rd, Blue=5th, Purple=Octave
 */

#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// === HARDWARE ===
DaisyPod hw;

// === DSP BLOCKS (from Block Diagram) ===
// pitch_shifter Voice A (+Interval)
PitchShifter ps_a;
// pitch_shifter Voice B (-Interval)
PitchShifter ps_b;

// === STATE ===
enum HarmonyPreset
{
    THIRDS,
    FIFTHS,
    OCTAVE,
    NUM_PRESETS
};
volatile HarmonyPreset current_preset = THIRDS;
volatile bool          bypass         = false;

// Harmony intervals in semitones
const float preset_intervals[NUM_PRESETS][2] = {
    {4.0f, -3.0f},  // Major 3rd up, Minor 3rd down
    {7.0f, -5.0f},  // Perfect 5th up, Perfect 4th down
    {12.0f, -12.0f} // Octave up, Octave down
};

// === CONTROL VALUES ===
volatile float dry_wet      = 0.5f;
volatile float detune_cents = 0.0f;

// === LED COLORS (RGB floats) ===
void SetPresetLED()
{
    switch(current_preset)
    {
        case THIRDS:
            hw.led1.Set(0.0f, 1.0f, 0.0f); // Green
            break;
        case FIFTHS:
            hw.led1.Set(0.0f, 0.0f, 1.0f); // Blue
            break;
        case OCTAVE:
            hw.led1.Set(1.0f, 0.0f, 1.0f); // Purple
            break;
        default: break;
    }
}

// === AUDIO CALLBACK ===
// Signal flow matches block diagram:
// AUDIO_IN -> [PS_A, PS_B] -> WET_MIX -> XFADE(dry, wet) -> AUDIO_OUT
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // MUST process analog controls strictly at audio block rate for proper filtering (BUG-005)
    hw.ProcessAnalogControls();

    // === KNOB INPUTS (from Block Diagram) ===
    // knob 1 -> Dry/Wet
    dry_wet = hw.knob1.Value();
    // knob 2 -> Detune (0-50 cents)
    detune_cents = hw.knob2.Value() * 50.0f;

    // === UPDATE PITCH SHIFTERS (from Block Diagram control connections) ===
    // B1 --(interval)--> PS_A, PS_B
    // K2 --(detune)--> PS_A, PS_B
    float interval_a = preset_intervals[current_preset][0];
    float interval_b = preset_intervals[current_preset][1];

    // Add detune offset (in semitones, cents/100)
    // Cast volatile float variables to float for parameter configuration
    ps_a.SetTransposition(interval_a + ((float)detune_cents / 100.0f));
    ps_b.SetTransposition(interval_b - ((float)detune_cents / 100.0f));

    // === AUDIO PROCESSING (from Block Diagram signal flow) ===
    for(size_t i = 0; i < size; i++)
    {
        // Mono input (sum L+R)
        float dry = (in[0][i] + in[1][i]) * 0.5f;

        if(bypass)
        {
            // Bypass: dry signal only
            out[0][i] = dry;
            out[1][i] = dry;
        }
        else
        {
            // pitch_shifter Voice A
            float wet_a = ps_a.Process(dry);
            // pitch_shifter Voice B
            float wet_b = ps_b.Process(dry);

            // WET_MIX = add(Voice A + Voice B)
            float wet_mix = (wet_a + wet_b) * 0.5f;

            // XFADE = crossfade(dry, wet, K1)
            // K1 = 0: 100% dry, K1 = 1: 100% wet
            float output = dry * (1.0f - (float)dry_wet) + wet_mix * (float)dry_wet;

            // Stereo output (mono-compatible)
            out[0][i] = output;
            out[1][i] = output;
        }
    }
}

int main(void)
{
    // Initialize Hardware
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sample_rate = hw.AudioSampleRate();

    // Initialize DSP Blocks (from Block Diagram)
    ps_a.Init(sample_rate);
    ps_b.Init(sample_rate);

    // Set initial transpositions
    ps_a.SetTransposition(preset_intervals[current_preset][0]);
    ps_b.SetTransposition(preset_intervals[current_preset][1]);

    // Set initial LED color
    SetPresetLED();
    hw.UpdateLeds();

    // Start Audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Main loop
    while(1)
    {
        hw.ProcessDigitalControls();

        // === BUTTON INPUTS (from Block Diagram) ===
        // button 1 -> Cycle Preset
        if(hw.button1.RisingEdge())
        {
            current_preset
                = static_cast<HarmonyPreset>((current_preset + 1) % NUM_PRESETS);
            SetPresetLED();
        }
        
        // button 2 -> Bypass Toggle
        if(hw.button2.RisingEdge())
        {
            bypass = !bypass;
            // Dim LED when bypassed
            if(bypass)
                hw.led1.Set(0.1f, 0.1f, 0.1f);
            else
                SetPresetLED();
        }

        hw.UpdateLeds();
        System::Delay(1);
    }
}
